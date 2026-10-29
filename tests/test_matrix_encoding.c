#include <criterion/criterion.h>

#include "../matrix_encoding.h"

// --- encode: known values ---------------------------------------------------

Test(encode, all_zero_bits_give_zero_matrix) {
  bool kk[ell] = {0};
  int K[mbar][nbar];
  for (int i = 0; i < mbar; i++)
    for (int j = 0; j < nbar; j++)
      K[i][j] = -1;
  encode(kk, K);
  for (int i = 0; i < mbar; i++)
    for (int j = 0; j < nbar; j++)
      cr_assert_eq(K[i][j], 0, "K[%d][%d] = %d, expected 0", i, j, K[i][j]);
}

Test(encode, all_one_bits_give_max_entry) {
  bool kk[ell];
  for (int p = 0; p < ell; p++) kk[p] = 1;
  int K[mbar][nbar] = {0};
  encode(kk, K);
  int expected = ((1 << B) - 1) << (D - B);
  for (int i = 0; i < mbar; i++)
    for (int j = 0; j < nbar; j++)
      cr_assert_eq(K[i][j], expected, "K[%d][%d] = %d, expected %d", i, j, K[i][j], expected);
}

Test(encode, single_bit_lands_in_correct_entry_and_weight) {
  for (int p = 0; p < ell; p++) {
    bool kk[ell] = {0};
    kk[p] = 1;
    int K[mbar][nbar] = {0};
    encode(kk, K);

    int entry = p / B, m = p % B;
    for (int i = 0; i < mbar; i++) {
      for (int j = 0; j < nbar; j++) {
        int expected = (i * nbar + j == entry) ? (1 << m) << (D - B) : 0;
        cr_assert_eq(K[i][j], expected, "bit %d set: K[%d][%d] = %d, expected %d", p, i, j,
                     K[i][j], expected);
      }
    }
  }
}

Test(encode, entries_are_in_range) {
  for (int k = 0; k < (1 << B); k++) {
    bool kk[ell];
    int K[mbar][nbar] = {0};
    for (int p = 0; p < ell; p++) kk[p] = (k >> (p % B)) & 1; // every entry is k
    encode(kk, K);
    for (int i = 0; i < mbar; i++)
      for (int j = 0; j < nbar; j++)
        cr_assert(K[i][j] >= 0 && K[i][j] < (1 << D), "k = %d: K[%d][%d] = %d not in [0, %d)", k,
                  i, j, K[i][j], (1 << D));
  }
}

// --- decode: known values and rounding --------------------------------------

Test(decode, exact_multiples_decode_to_entry_value) {
  for (int k = 0; k < (1 << B); k++) {
    int K[mbar][nbar];
    bool expected[ell], out[ell] = {0};
    for (int i = 0; i < mbar; i++) {
      for (int j = 0; j < nbar; j++) {
        K[i][j] = k << (D - B);
      }
    }

    for (int p = 0; p < ell; p++) {
      expected[p] = (k >> (p % B)) & 1; // every entry is k
    }
    decode(K, out);
    for (int p = 0; p < ell; p++)
      cr_assert_eq(out[p], expected[p], "k = %d, bit %d: decoded %d, expected %d", k, p, out[p],
                   expected[p]);
  }
}

// An entry encoding k decodes back to k for any error e with
// -2^((D - B)-1) <= e < 2^((D - B)-1), and rounds to k + 1 at e = 2^((D - B)-1).
Test(decode, rounds_to_nearest_with_wraparound) {
  const int half = 1 << ((D - B) - 1);
  const struct { int offset, delta; } cases[] = {
      {half - 1, 0}, // just below the upper boundary
      {-half, 0},    // lower boundary, rounds up to k
      {half, 1},     // upper boundary, rounds up to k + 1
  };
  for (int k = 0; k < (1 << B); k++) {
    for (size_t c = 0; c < sizeof cases / sizeof cases[0]; c++) {
      int value = ((k << (D - B)) + cases[c].offset + (1 << D)) % (1 << D);
      int want = (k + cases[c].delta) % (1 << B);

      int K[mbar][nbar] = {0};
      bool expected[ell], out[ell] = {0};
      K[0][0] = value;
      for (int p = 0; p < ell; p++) expected[p] = p < B ? (want >> p) & 1 : 0; // entry 0 only
      decode(K, out);
      for (int p = 0; p < ell; p++)
        cr_assert_eq(out[p], expected[p],
                     "K[0][0] = %d (k = %d, offset %d): bit %d decoded %d, expected %d", value, k,
                     cases[c].offset, p, out[p], expected[p]);
    }
  }
}

Test(decode, output_is_bits) {
  const int samples[] = {0, 1, 31, 32, 33, 12345, (1 << D) / 2, (1 << D) - 33, (1 << D) - 1};
  for (size_t s = 0; s < sizeof samples / sizeof samples[0]; s++) {
    int K[mbar][nbar];
    bool out[ell];
    for (int i = 0; i < mbar; i++)
      for (int j = 0; j < nbar; j++)
        K[i][j] = samples[s];
    for (int p = 0; p < ell; p++) out[p] = -1;
    decode(K, out);
    for (int p = 0; p < ell; p++)
      cr_assert(out[p] == 0 || out[p] == 1, "K = %d: bit %d is %d", samples[s], p, out[p]);
  }
}

// --- round trip -------------------------------------------------------------

Test(roundtrip, fixed_patterns) {
  {
    bool kk[ell], out[ell] = {0};
    int K[mbar][nbar] = {0};
    for (int p = 0; p < ell; p++) kk[p] = 0;
    encode(kk, K);
    decode(K, out);
    for (int p = 0; p < ell; p++)
      cr_assert_eq(out[p], kk[p], "all zeros, bit %d: decoded %d, expected %d", p, out[p], kk[p]);
  }
  {
    bool kk[ell], out[ell] = {0};
    int K[mbar][nbar] = {0};
    for (int p = 0; p < ell; p++) kk[p] = 1;
    encode(kk, K);
    decode(K, out);
    for (int p = 0; p < ell; p++)
      cr_assert_eq(out[p], kk[p], "all ones, bit %d: decoded %d, expected %d", p, out[p], kk[p]);
  }
  {
    bool kk[ell], out[ell] = {0};
    int K[mbar][nbar] = {0};
    for (int p = 0; p < ell; p++) kk[p] = p % 2;
    encode(kk, K);
    decode(K, out);
    for (int p = 0; p < ell; p++)
      cr_assert_eq(out[p], kk[p], "alternating, bit %d: decoded %d, expected %d", p, out[p],
                   kk[p]);
  }
  {
    bool kk[ell], out[ell] = {0};
    int K[mbar][nbar] = {0};
    for (int p = 0; p < ell; p++) kk[p] = (p * 7 + 3) % 5 < 2; // irregular pattern
    encode(kk, K);
    decode(K, out);
    for (int p = 0; p < ell; p++)
      cr_assert_eq(out[p], kk[p], "irregular, bit %d: decoded %d, expected %d", p, out[p], kk[p]);
  }
}

Test(roundtrip, every_single_bit) {
  for (int b = 0; b < ell; b++) {
    bool kk[ell] = {0}, out[ell] = {0};
    int K[mbar][nbar] = {0};
    kk[b] = 1;
    encode(kk, K);
    decode(K, out);
    for (int p = 0; p < ell; p++)
      cr_assert_eq(out[p], kk[p], "bit %d set: bit %d decoded %d, expected %d", b, p, out[p],
                   kk[p]);
  }
}

// Every B-bit value in every entry: 2^B cases, so this is exhaustive per entry.
Test(roundtrip, every_entry_value) {
  for (int k = 0; k < (1 << B); k++) {
    bool kk[ell], out[ell] = {0};
    int K[mbar][nbar] = {0};
    for (int p = 0; p < ell; p++) kk[p] = (k >> (p % B)) & 1; // every entry is k
    encode(kk, K);
    decode(K, out);
    for (int p = 0; p < ell; p++)
      cr_assert_eq(out[p], kk[p], "k = %d, bit %d: decoded %d, expected %d", k, p, out[p], kk[p]);
  }
}
