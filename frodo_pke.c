#include "frodo_pke.h"
#include "matrix_generation.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fips202.h"
#include "linalg.h"
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
  bool salt[8 + len_seed_SE];
  for (int i = 0; i < 8; i++) salt[i] = (0x5F >> i) & 1;
  for (int i = 8; i < 8 + len_seed_SE; i++) salt[i] = seed_SE[i - 8];

  uint8_t in[(8 + len_seed_SE + 7) / 8];
  bits_to_bytes(8 + len_seed_SE, salt, in);

  uint8_t out[(2 * n * nbar * len_chi) / 8];
  shake128(out, sizeof out, in, sizeof in);

  bool rr[2 * n * nbar][len_chi];
  bytes_to_bits(2 * n * nbar * len_chi, out, rr[0]);

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
  memcpy(kp.pk.fst, seed_A, sizeof seed_A);
  memcpy(kp.pk.snd, B, sizeof B);
  memcpy(kp.sk, ST, sizeof ST);
  return kp;
}
