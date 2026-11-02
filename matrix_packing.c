/**
 * 2.2.2 Packing matrices modulo q
 *
 * Functions to pack and unpack mod-q integer matrices as bit strings.
 */

#include "matrix_packing.h"

/**
 * Index into bb of the p-th bit of the packed stream.
 *
 * The spec concatenates the D-bit coefficients MSB first, and the reference implementation (and so the KATs) writes
 * that stream into bytes MSB first. bits_to_bytes places bit 8k in the LSB of byte k, so bit p of the stream must be
 * stored at index p ^ 7 (reversing each group of 8) for the serialised bytes to match. This requires
 * D * n_1 * n_2 to be a multiple of 8, which holds since nbar = 8.
 */
static size_t packed_index(const size_t p) {
  return p ^ 7;
}

/**
 * Pack an (@param n_1)-by-(@param n_2) matrix @param C with entries in
 * Z_q into a bit string by concatenating the D-bit matrix coefficients.
 *
 * Writes output to buffer @param bb.
 */
void pack(const size_t n_1, const size_t n_2, int C[n_1][n_2], bool bb[D * n_1 * n_2]) {
  for (int i = 0; i < n_1; i++) {
    for (int j = 0; j < n_2; j++) {
      for (int l = 0; l < D; l++) {
        bb[packed_index((i * n_2 + j) * D + l)] = C[i][j] >> (D - 1 - l) & 1;
      }
    }
  }
}

/**
 * The inverse of pack.
 */
void unpack(const size_t n_1, const size_t n_2, const bool bb[D * n_1 * n_2], int C[n_1][n_2]) {
  for (int i = 0; i < n_1; i++) {
    for (int j = 0; j < n_2; j++) {
      C[i][j] = 0;
      for (int l = 0; l < D; l++) {
        C[i][j] += bb[packed_index((i * n_2 + j) * D + l)] * (1 << (D - 1 - l));
      }
    }
  }
}
