#include "matrix_generation.h"

#include <stdint.h>
#include <string.h>

#include "params.h"
#include "utils.h"

/**
 * Algorithm 8: Frodo.Gen using SHAKE128
 *
 * Generate a pseudorandom matrix A ∈ Z_q^n×n.
 *
 * For each row index i = 0, 1, ..., n - 1, it calls SHAKE128 with a main input of seed_A, prefixed with a counter
 * (represented as a 16-bit integer in little-endian byte order), to produce a 16n-bit output string.
 *
 * It splits this output into 16-bit integers (in little-endian byte order) c_{i,j} for j = 0, 1, ..., n -1, and sets
 * A_{i,j} = c_{i,j} mod q for all j.
 *
 * @param seed_A  The random seed from which to generate the pseudorandom matrix entries.
 * @param A       The output buffer, a newly generated pseudorandom matrix.
 */
void gen_shake128(const bool seed_A[len_seed_A], int A[n][n]) {
  for (uint16_t i = 0; i < n; i++) {
    // Generate a pseudorandom matrix row: SHAKE128(<i>_16 || seed_A, 16n)
    bool seed[16 + len_seed_A];
    bytes_to_bits(16, (const uint8_t[]){i & 0xFF, i >> 8}, seed);
    memcpy(seed + 16, seed_A, len_seed_A * sizeof(bool));
    bool c[16 * n];
    shake128_bits(16 + len_seed_A, seed, 16 * n, c);

    // Each c_ij is a 16-bit little-endian integer
    for (int j = 0; j < n; j++) {
      int c_ij = 0;

      for (int k = 16 * j; k < 16 * j + 16; k++) {
        c_ij += (1 << (k - 16 * j)) * c[k];
      }

      A[i][j] = c_ij % q;
    }
  }
}