#ifndef FRODOKEM_PARAS_H
#define FRODOKEM_PARAS_H
#include <stddef.h>

// Params set for Frodo-640

constexpr int len_chi = 16;
constexpr int s = 12;    // The support of chi is S_chi = {-s, ..., s}.
constexpr int D = 15;
constexpr int q = 32768;  // A power-of-two integer modulus with exponent D <= 16.
constexpr int n = 640;    // Integer matrix dimension, n ≡ 0 (mod 8).
constexpr int mbar = 8;   // Integer matrix dimension.
constexpr int nbar = 8;   // Integer matrix dimension, the number of samples.
constexpr int B = 2;      // B <= D, the number of bits encoded in each matrix entry.
constexpr int ell = B * mbar * nbar; // The length of bit strings that are encoded as mbar-by-nbar matrices.
constexpr int len_mu = ell;
constexpr size_t len_seed_A = 128;
constexpr size_t len_seed_SE = 128;
constexpr size_t len_s = 128;
constexpr size_t len_pkh = 128;
constexpr size_t len_z = 128;
constexpr size_t len_b = D * n * nbar;
constexpr size_t len_ss = 128;
constexpr size_t len_pk = len_seed_A + len_b;
constexpr size_t len_k = 128;
constexpr size_t len_c1 = D * mbar * n;
constexpr size_t len_c2 = D * mbar * nbar;

// The distribution table T_chi = (T_chi(0), ..., T_chi(s)) for chi_Frodo-640, derived from Table 3's probabilities
// (in multiples of 2^-16) chi(0..12) = 9288, 8720, 7216, 5264, 3384, 1918, 958, 422, 164, 56, 17, 4, 1 via
// T_chi(0) = 2^(len_chi - 1) * chi(0) - 1 and T_chi(z) = T_chi(0) + 2^len_chi * sum_{i=1}^{z} chi(i).
static const int T_chi[s + 1] = { 4643, 13363, 20579, 25843, 29227, 31145, 32103, 32525, 32689, 32745, 32762, 32766, 32767 };

#endif //FRODOKEM_PARAS_H