#include "frodo_pke.h"
#include "matrix_generation.h"

#include <stdlib.h>

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

key_pair key_gen() {
  // Choose a uniformly random seed_A
  bool seed_A[len_seed_A];
  for (int i = 0; i < len_seed_A; i++) {
    const int x = rand();
    if (x > RAND_MAX) {
      seed_A[i] = 1;
    } else {
      seed_A[i] = 0;
    }
  }

  // Generate the matrix A via Frodo.Gen(seed_A)
  int A[n][n];
  gen_shake128(seed_A, A);

  // Choose a uniformly random seed_SE
  bool seed_SE[len_seed_SE];
  for (int i = 0; i < len_seed_A; i++) {
    const int x = rand();
    if (x > RAND_MAX) {
      seed_A[i] = 1;
    } else {
      seed_A[i] = 0;
    }
  }

  // Choose a pseudorandom bit string
  
}