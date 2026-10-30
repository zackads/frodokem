#ifndef FRODOKEM_FRODO_KEM_H
#define FRODOKEM_FRODO_KEM_H
#include "params.h"

struct kem_sk {
  const bool s[len_s];
  const bool seed_A[len_seed_A];
  const bool b[len_b];
  const int S_T[nbar][n];
  const bool pkh[len_pkh];
};

typedef struct {
  const bool pk[len_seed_A + len_b];
  const struct kem_sk sk;
} kem_keypair;

typedef struct {
  const bool ciphertext[(mbar * n + mbar * nbar) * D];
  const bool ss[len_ss];
} kem_encaps_result;

kem_keypair keygen(void);
kem_encaps_result encaps(bool pk[len_seed_A + len_b]);
void decaps(const bool ciphertext[len_c1 + len_c2], const struct kem_sk *sk, bool ss[len_ss]);


#endif //FRODOKEM_FRODO_KEM_H
