/**
 * 2.2.1 Matrix encoding of bit strings
 */

#ifndef MATRIX_ENCODING_H
#define MATRIX_ENCODING_H

constexpr int D = 16;
constexpr int q = 1 << D; // A power-of-two integer modulus with exponent D <= 16.
constexpr int n = 4;      // Integer matrix dimension, n ≡ 0 (mod 8).
constexpr int mbar = 4;   // Integer matrix dimension.
constexpr int nbar = 4;   // Integer matrix dimension.
constexpr int B = 10;     // B <= D, the number of bits encoded in each matrix entry.
constexpr int ell = B * mbar * nbar; // The length of bit strings that are encoded as mbar-by-nbar matrices.

void encode(const int kk[ell], int K[mbar][nbar]);
void decode(const int K[mbar][nbar], int kk[ell]);

#endif
