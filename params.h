#ifndef FRODOKEM_PARAS_H
#define FRODOKEM_PARAS_H
#include <stddef.h>

constexpr int len_chi = 12;
constexpr int s = 42;
constexpr int D = 16;
constexpr int q = 1 << D; // A power-of-two integer modulus with exponent D <= 16.
constexpr int n = 4;      // Integer matrix dimension, n ≡ 0 (mod 8).
constexpr int mbar = 4;   // Integer matrix dimension.
constexpr int nbar = 4;   // Integer matrix dimension.
constexpr int B = 10;     // B <= D, the number of bits encoded in each matrix entry.
constexpr int ell = B * mbar * nbar; // The length of bit strings that are encoded as mbar-by-nbar matrices.

#endif //FRODOKEM_PARAS_H
