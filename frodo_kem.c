#include "frodo_kem.h"

#include <string.h>

#include "frodo_pke.h"
#include "linalg.h"
#include "matrix_generation.h"
#include "matrix_packing.h"
#include "sampling.h"
#include "utils.h"

kem_keypair keygen() {
  // 1. Choose uniformly random seeds
  bool s[len_s];
  bool seed_SE[len_seed_SE];
  bool z[len_z];
  dodgy_uniform_random_bits(len_s, s);
  dodgy_uniform_random_bits(len_seed_SE, seed_SE);
  dodgy_uniform_random_bits(len_z, z);

  // 2.  Generate pseudorandom seed
  bool seed_A[len_seed_A];
  shake128_bits(len_z, z, len_seed_A, seed_A);

  // 3.  Generate the matrix A
  int A[n][n];
  gen_shake128(seed_A, A);

  // 4.  Generate the pseudorandom bit string
  bool seed1[8 + len_seed_SE];
  bytes_to_bits(8, (const uint8_t[]){ 0x5F }, seed1); // Write the 0x5F prefix to the first 8 bits of seed
  memcpy(seed1 + 8, seed_SE, sizeof seed_SE);
  bool rr[2 * n * nbar][len_chi];
  shake128_bits(8 + len_seed_SE, seed1, 2 * n * nbar * len_chi, rr[0]);

  // 5.  Sample error matrix S^T
  int S_T[nbar][n];
  sample_matrix(nbar, n, &rr[0], S_T);

  // 6.  Sample error matrix E
  int E[n][nbar];
  sample_matrix(n, nbar, &rr[n * nbar], E);

  // 7.  Compute B = AS + E
  int S[n][nbar];
  transpose(nbar, n, S_T, S);
  int AS[n][nbar];
  matrix_mult(n, n, nbar, A, S, AS);
  int B[n][nbar];
  matrix_add(n, nbar, AS, E, B);

  // 8.  Compute b = Frodo.Pack(B)
  bool b[len_b];
  pack(n, nbar, B, b);

  // 9.  Compute pkh = SHAKE(seed_A || b, len_pkh)
  bool pk[len_seed_A + len_b];
  memcpy(pk, seed_A, sizeof seed_A);
  memcpy(pk + len_seed_A, b, sizeof b);
  bool pkh[len_pkh];
  shake128_bits(len_seed_A + len_b, pk, len_pkh, pkh);

  // Return public key pk <- seed_A || b and secret key sk' <- (s || seed_A || b, S^T, pkh)
  kem_keypair kp;
  memcpy(kp.pk, pk, sizeof pk);
  memcpy(kp.sk.s, s, sizeof s);
  memcpy(kp.sk.seed_A, seed_A, sizeof seed_A);
  memcpy(kp.sk.b, b, sizeof b);
  memcpy(kp.sk.S_T, S_T, sizeof S_T);
  memcpy(kp.sk.pkh, pkh, sizeof pkh);
  return kp;
}

kem_encaps_result encaps(bool pk[len_seed_A + len_b]) {
  // 1.  Choose a uniformly random key
  bool mu[len_mu];
  dodgy_uniform_random_bits(len_mu, mu);

  // 2.  Compute pkh
  bool pkh[len_pkh];
  shake128_bits(len_pk, pk, len_pkh, pkh);

  // 3.  Generate pseudorandom values seed_SE || k <- SHAKE(pkh || mu, len_seed_SE + len_k)
  bool seed1[len_pkh + len_mu];
  memcpy(seed1, pkh, sizeof pkh);
  memcpy(seed1 + len_pkh, mu, sizeof mu);
  bool seed_SE_k[len_seed_SE + len_k];
  shake128_bits(len_pkh + len_mu, seed1, len_seed_SE + len_k, seed_SE_k);

  // 4.  Generate pseudorandom bit string
  bool rr[2 * mbar * n + mbar * nbar][len_chi];
  bool seed2[8 + len_seed_SE];
  bytes_to_bits(8, (const uint8_t[]){ 0x96 }, seed2); // Write the 0x96 prefix to the first 8 bits of seed
  memcpy(seed2 + 8, seed_SE_k, len_seed_SE * sizeof(bool));
  shake128_bits(8 + len_seed_SE, seed2, (2 * mbar * n + mbar * nbar) * len_chi, rr[0]);

  // 5.  Sample error matrix S'
  int S_prime[mbar][n];
  sample_matrix(mbar, n, rr, S_prime);

  // 6.  Sample error matrix E'
  int E_prime[mbar][n];
  sample_matrix(mbar, n, rr + mbar * n, E_prime);

  // 7.  Compute A <- Frodo.Gen(seed_A)
  int A[n][n];
  gen_shake128(pk, A);

  // 8.  Compute B' <- S'A + E'
  int B_prime[mbar][n];
  matrix_mult(mbar, n, n, S_prime, A, B_prime);
  matrix_add(mbar, n, B_prime, E_prime, B_prime);

  // 9.  Compute c_1 <- Frodo.Pack(B')
  bool c_1[len_c1];
  pack(mbar, n, B_prime, c_1);

  // 10.  Sample error matrix E''
  int E_double_prime[mbar][nbar];
  sample_matrix(mbar, nbar, rr + 2 * mbar * n, E_double_prime);

  // 11.  Compute B <- Frodo.Unpack(b, n, nbar)
  int B[n][nbar];
  unpack(n, nbar, pk + len_seed_A, B);

  // 12.  Compute V <- S'B + E''
  int V[mbar][nbar];
  matrix_mult(mbar, n, nbar, S_prime, B, V);
  matrix_add(mbar, nbar, V, E_double_prime, V);

  // 13.  Compute C <- V + Frodo.Encode(mu)
  int C[mbar][nbar];
  int Mu[mbar][nbar];
  encode(mu, Mu);
  matrix_add(mbar, nbar, V, Mu, C);

  // 14.  Compute c_2 <- Frodo.Pack(C)
  bool c_2[len_c2];
  pack(mbar, nbar, C, c_2);

  // 15.  Compute ss <- SHAKE(c_1 || c_2 || k, len_ss)
  bool seed3[len_c1 + len_c2 + len_k];
  memcpy(seed3, c_1, sizeof(c_1));
  memcpy(seed3 + len_c1, c_2, sizeof(c_2));
  memcpy(seed3 + len_c1 + len_c2, seed_SE_k + len_seed_SE, len_k * sizeof(bool));
  bool ss[len_ss];
  shake128_bits(len_c1 + len_c2 + len_k, seed3, len_ss, ss);

  kem_encaps_result res;
  memcpy(res.ciphertext, c_1, sizeof(c_1));
  memcpy(res.ciphertext + len_c1, c_2, sizeof(c_2));
  memcpy(res.ss, ss, sizeof(ss));

  return res;
}

void decaps(const bool ciphertext[len_c1 + len_c2], const struct kem_sk *sk, bool ss[len_ss]) {
  // 1.  Compute B' <- Frodo.Unpack(c_1, mbar, n)
  int B_prime[mbar][n];
  unpack(mbar, n, ciphertext, B_prime);

  // 2.  Compute C <- Frodo.Unpack(c_2, mbar, nbar)
  int C[mbar][nbar];
  unpack(mbar, nbar, ciphertext + len_c1, C);

  // 3.  Compute M <- C - B'S
  int S[n][nbar];
  transpose(nbar, n, (int (*)[n])sk->S_T, S);
  int neg_B_prime_S[mbar][nbar];
  matrix_mult(mbar, n, nbar, B_prime, S, neg_B_prime_S);
  matrix_scale(mbar, nbar, -1, neg_B_prime_S, neg_B_prime_S);
  int M[mbar][nbar];
  matrix_add(mbar, nbar, C, neg_B_prime_S, M);

  // 4.  Compute mu' <- Frodo.Decode(M)
  bool mu_prime[len_mu];
  decode(M, mu_prime);

  // 5.  Parse pk <- seed_A || b
  bool pk[len_pk];
  memcpy(pk, sk->seed_A, sizeof sk->seed_A);
  memcpy(pk + len_seed_A, sk->b, sizeof sk->b);

  // 6.  Generate pseudorandom values seed_SE' || k' <- SHAKE(pkh || mu', len_seed_SE + len_k)
  bool seed1[len_pkh + len_mu];
  memcpy(seed1, sk->pkh, sizeof sk->pkh);
  memcpy(seed1 + len_pkh, mu_prime, sizeof mu_prime);
  bool seed_SE_k_prime[len_seed_SE + len_k];
  shake128_bits(len_pkh + len_mu, seed1, len_seed_SE + len_k, seed_SE_k_prime);

  // 7.  Generate pseudorandom bit string
  bool rr[2 * mbar * n + mbar * nbar][len_chi];
  bool seed2[8 + len_seed_SE];
  bytes_to_bits(8, (const uint8_t[]){ 0x96 }, seed2); // Write the 0x96 prefix to the first 8 bits of seed
  memcpy(seed2 + 8, seed_SE_k_prime, len_seed_SE * sizeof(bool));
  shake128_bits(8 + len_seed_SE, seed2, (2 * mbar * n + mbar * nbar) * len_chi, rr[0]);

  // 8.  Sample error matrix S'
  int S_prime[mbar][n];
  sample_matrix(mbar, n, rr, S_prime);

  // 9.  Sample error matrix E'
  int E_prime[mbar][n];
  sample_matrix(mbar, n, rr + mbar * n, E_prime);

  // 10.  Generate A <- Frodo.Gen(seed_A)
  int A[n][n];
  gen_shake128(sk->seed_A, A);

  // 11.  Compute B'' <- S'A + E'
  int B_double_prime[mbar][n];
  matrix_mult(mbar, n, n, S_prime, A, B_double_prime);
  matrix_add(mbar, n, B_double_prime, E_prime, B_double_prime);

  // 12.  Sample error matrix E''
  int E_double_prime[mbar][nbar];
  sample_matrix(mbar, nbar, rr + 2 * mbar * n, E_double_prime);

  // 13.  Compute B <- Frodo.Unpack(b, n, nbar)
  int B[n][nbar];
  unpack(n, nbar, sk->b, B);

  // 14.  Compute V <- S'B + E''
  int V[mbar][nbar];
  matrix_mult(mbar, n, nbar, S_prime, B, V);
  matrix_add(mbar, nbar, V, E_double_prime, V);

  // 15.  Compute C' <- V + Frodo.Encode(mu')
  int C_prime[mbar][nbar];
  int Mu_prime[mbar][nbar];
  encode(mu_prime, Mu_prime);
  matrix_add(mbar, nbar, V, Mu_prime, C_prime);

  // 16.  (in constant time) k_bar <- k' if (B' || C = B'' || C') else k_bar <- s
  // NOTE: For simplicity this is NOT constant time: memcmp may return early and the branch below depends on the
  // result, leaking timing information about whether the ciphertext was valid. A real implementation must avoid this.
  bool k_bar[len_k];
  if (memcmp(B_prime, B_double_prime, sizeof B_prime) == 0 && memcmp(C, C_prime, sizeof C) == 0) {
    memcpy(k_bar, seed_SE_k_prime + len_seed_SE, sizeof k_bar);
  } else {
    memcpy(k_bar, sk->s, sizeof k_bar);
  }

  // 17.  Compute ss <- SHAKE(c_1 || c_2 || k_bar, len_ss)
  bool seed3[len_c1 + len_c2 + len_k];
  memcpy(seed3, ciphertext, (len_c1 + len_c2) * sizeof(bool));
  memcpy(seed3 + len_c1 + len_c2, k_bar, sizeof(k_bar));
  shake128_bits(len_c1 + len_c2 + len_k, seed3, len_ss, ss);
}
