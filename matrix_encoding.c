/**
 * 2.2.1 Matrix encoding of bit strings
 *
 * Functions to encode and decode bit strings as mod-q integer matrices.
 */

#include "matrix_encoding.h"

/**
 * Encode a bit string @param kk of length ell as an mbar-by-nbar matrix
 * with entries in Z_q by applying ec(k) := k·q/2^B to B-bit substrings
 * suquentially and filling the matrix row by row entry-wise.
 *
 * Each B-bit substring of @param kk is interpreted as an integer
 * 0 <= k < 2^B and then encoded with ec(k), which means that B-bit values
 * are placed into the B most significant bits of the corresponding entry
 * modulo q.
 *
 * Writes output to buffer @param K.
 */
void encode(const int kk[ell], int K[mbar][nbar]) {
  for (int i = 0; i < mbar; i++) {
    for (int j = 0; j < nbar; j++) {
      int k = 0;
      for (int l = 0; l < B; l++) {
        k += kk[((i * nbar + j) * B + l)] * (1 << l);
      }
      K[i][j] = k * q / (1 << B);
    }
  }
}

/**
 * Decodes the mbar-by-nbar matrix @param K into a bit string of length
 * ell, extracting B bits from each entry by applying the function
 * dc(c) := round(c · 2^B / q) mod 2^B.
 *
 * That is, the Z_q-entry is interpreted as an integer, then divided by
 * q/2^B and rounded.  This amounts to rounding the B most significant
 * bits of each entry.
 *
 * Writes output to buffer @param kk.
 */
void decode(const int K[mbar][nbar], int kk[ell]) {
  for (int i = 0; i < mbar; i++) {
    for (int j = 0; j < nbar; j++) {
      const int k = ((K[i][j] << B) + q / 2) / q % (1 << B);
      for (int l = 0; l < B; l++) {
        kk[(i * nbar + j) * B + l] = k >> l & 1;
      }
    }
  }
}