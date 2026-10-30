#ifndef FRODOKEM_FRODO_PKE_H
#define FRODOKEM_FRODO_PKE_H

#include "params.h"

typedef struct {
  bool seed_A[len_seed_A];
  int B[n][nbar];
} pk_t;

typedef int sk_t[nbar][n];

typedef struct {
  pk_t pk;
  sk_t sk;
} key_pair;

typedef struct {
  int C_1[mbar][n];
  int C_2[mbar][nbar];
} ciphertext;

key_pair gen();
ciphertext enc(bool mu[len_mu], pk_t pk);
void dec(ciphertext c, sk_t sk, bool mu_prime[len_mu]);

#endif //FRODOKEM_FRODO_PKE_H
