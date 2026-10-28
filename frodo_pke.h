#ifndef FRODOKEM_FRODO_PKE_H
#define FRODOKEM_FRODO_PKE_H

#include "params.h"

typedef struct {
  bool fst[len_seed_A];
  int snd[n][nbar];
} pk_t;

typedef int sk_t[nbar][n];

typedef struct {
  pk_t pk;
  sk_t sk;
} key_pair;

key_pair key_gen();

#endif //FRODOKEM_FRODO_PKE_H
