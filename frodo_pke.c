#include "frodo_pke.h"
#include "matrix_generation.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fips202.h"
#include "linalg.h"
#include "matrix_encoding.h"
#include "params.h"
#include "sampling.h"
#include "utils.h"

/**
 * Algorithm 9: FrodoPKE.KeyGen
 *
 * @return Key pair (pk, sk) ∈ ({0,1}^len_seed_A × Z_q^{n×nbar}) × Z_q^{nbar×n}.
 */
key_pair key_gen() {
  // Choose a uniformly random seed_A
  uint8_t seed_A_bytes[len_seed_A / 8];
  arc4random_buf(seed_A_bytes, sizeof seed_A_bytes);
  bool seed_A[len_seed_A];
  bytes_to_bits(len_seed_A, seed_A_bytes, seed_A);

  // Generate the matrix A via Frodo.Gen(seed_A)
  int A[n][n];
  gen_shake128(seed_A, A);

  // Choose a uniformly random seed_SE
  uint8_t seed_SE_bytes[len_seed_SE / 8];
  arc4random_buf(seed_SE_bytes, sizeof seed_SE_bytes);
  bool seed_SE[len_seed_SE];
  bytes_to_bits(len_seed_SE, seed_SE_bytes, seed_SE);

  // Generate pseudorandom bit string (r^(0), ..., r^(2n·nbar - 1)) <- SHAKE(0x5F || seed_SE, 2n·nbar·len_chi)
  bool rr[2 * n * nbar][len_chi];
  shake128_bits(8, 0x5F, len_seed_SE, seed_SE, 2 * n * nbar * len_chi, rr[0]);

  // Sample error matrix S^T <- Frodo.SampleMatrix((r^(0), ..., r^(n·nbar - 1)), nbar, n, T_chi)
  const bool (*rr_ST)[len_chi] = &rr[0];
  int ST[nbar][n];
  sample_matrix(nbar, n, rr_ST, ST);

  int S[n][nbar];
  transpose(nbar, n, ST, S);

  // Sample error matrix E <- Frodo.SampleMatrix((r^(n·nbar), ..., r^(2n·nbar - 1)), n, nbar, T_chi)
  const bool (*rr_E)[len_chi] = &rr[n * nbar];
  int E[n][nbar];
  sample_matrix(n, nbar, rr_E, E);

  // Compute B = AS + E (the LWE sample)
  int AS[n][nbar];
  matrix_mult(n, n, nbar, A, S, AS);
  int B[n][nbar];
  matrix_add(n, nbar, AS, E, B);

  // Return public key pk <- (seed_A, B) and secret key sk <- S^T
  key_pair kp;
  memcpy(kp.pk.seed_A, seed_A, sizeof seed_A);
  memcpy(kp.pk.B, B, sizeof B);
  memcpy(kp.sk, ST, sizeof ST);
  return kp;
}

ciphertext enc(bool mu[len_mu], pk_t pk) {
  // 1. Generate A <-- Frodo.Gen(seed_A)
  int A[n][n];
  gen_shake128(pk.seed_A, A);

  // 2. Choose a uniformly random seed
  uint8_t seed_SE_bytes[len_seed_SE / 8];
  arc4random_buf(seed_SE_bytes, sizeof seed_SE_bytes);
  bool seed_SE[len_seed_SE];
  bytes_to_bits(len_seed_SE, seed_SE_bytes, seed_SE);

  // 3. Generate pseudorandom bit string
  constexpr size_t n_samples = 2 * mbar * n + mbar * nbar;
  bool rr[n_samples][len_chi];
  shake128_bits(8, 0x96, len_seed_SE, seed_SE, n_samples * len_chi, &rr[0][0]);

  // 4. Sample error matrix S'
  int S_prime[mbar][n];
  sample_matrix(mbar, n, rr, S_prime);

  // 5. Sample error matrix E'
  int E_prime[mbar][n];
  sample_matrix(mbar, n, rr + mbar * n, E_prime);

  // 6. Sample error matrix E''
  int E_double_prime[mbar][nbar];
  sample_matrix(mbar, nbar, rr + 2 * mbar * n, E_double_prime);

  ciphertext ct;

  // C_1 = B' = S'A + E'
  int B_prime[mbar][n];
  matrix_mult(mbar, n, n, S_prime, A, B_prime);
  matrix_add(mbar, n, B_prime, E_prime, ct.C_1);

  // V = S'B + E''
  int V[mbar][nbar];
  matrix_mult(mbar, n, nbar, S_prime, pk.B, V);
  matrix_add(mbar, nbar, V, E_double_prime, V);

  // C_2 = V + Frodo.Encode(mu)
  encode(mu, ct.C_2);
  matrix_add(mbar, nbar, ct.C_2, V, ct.C_2);

  return ct;
}

void dec(ciphertext c, sk_t sk, bool mu_prime[len_mu]) {
  // sk = S^T, so we transpose to get S
  int S[n][nbar];
  transpose(nbar, n, sk, S);

  int neg_C_1_S[mbar][nbar];
  matrix_mult(mbar, n, nbar, c.C_1, S, neg_C_1_S);
  matrix_scale(mbar, nbar, -1, neg_C_1_S, neg_C_1_S);

  int M[mbar][nbar];
  matrix_add(mbar, nbar, c.C_2, neg_C_1_S, M);

  decode(M, mu_prime);
}
