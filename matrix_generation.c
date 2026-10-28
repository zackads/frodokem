#include "matrix_generation.h"

#include <stdint.h>
#include <string.h>

#include "fips202.h"
#include "params.h"

/**
 * Algorithm 8: Frodo.Gen using SHAKE128
 *
 * @param n
 * @param seed_A
 * @param A
 */
void gen_shake128(const bool seed_A[len_seed_A], int A[n][n]) {
  uint8_t c[n][16 * n];
  memset(c, 0, sizeof c); // Initialise every cell to 0.

  for (uint16_t i = 0; i < n; i++) {
    uint8_t bb[16 + len_seed_A];

    // Write i to the first 16 bits of bb
    for (int j = 0; j < 16; j++) {
      bb[j] = i >> j & 1;
    }

    // Write seed_A to the last len_seed_A bits of bb
    for (int j = 16; j < len_seed_A; j++) {
      bb[j] = seed_A[j];
    }

    shake128(c[i], 16 * n, bb, 16 + len_seed_A);

    for (int j = 0; j < n; j++) {
      int c_ij = 0;
      for (int k = 16 * j; k < 16 * j + 15; k++) {
        c_ij += (1 << (k - 16 * j)) * c[i][k];
      }

      A[i][j] = c_ij % q;
    }
  }
}