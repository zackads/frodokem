#include <criterion/criterion.h>

#include "../matrix_packing.h"

// The packed stream is the D-bit big-endian coefficients concatenated row by row, written into bytes MSB first.
// Bit p of the stream therefore lives at bb[p ^ 7], so that bits_to_bytes (LSB first) produces the reference bytes.
// Packed lengths must be a multiple of 8, so every matrix here has n_1 * n_2 ≡ 0 (mod 8).
#define PI(p) ((p) ^ 7)

// --- pack: known values -----------------------------------------------------

// Entries are written as D-bit big-endian strings; the first entry of a 1x8 matrix occupies stream bits 0..D-1.
Test(pack, single_entry_is_big_endian) {
  const int values[] = {0, 1, 2, 1 << (D - 1), 0xA5C3 & ((1 << D) - 1), (1 << D) - 1};
  for (size_t v = 0; v < sizeof values / sizeof values[0]; v++) {
    int C[1][8] = {{values[v]}};
    bool bb[D * 8];
    pack(1, 8, C, bb);
    for (int l = 0; l < D; l++) {
      bool expected = (values[v] >> (D - 1 - l)) & 1;
      cr_assert_eq(bb[PI(l)], expected, "C = 0x%x: bit %d = %d, expected %d", values[v], l, bb[PI(l)],
                   expected);
    }
  }
}

Test(pack, entries_are_concatenated_row_by_row) {
  enum { R = 2, S = 4 };
  const int mask = (1 << D) - 1; // Keep every entry a valid D-bit value
  int C[R][S] = {{0x0001, 1 << (D - 1), 0x00FF, 0x5A5A & mask}, {0xFF00 & mask, 0x1234, mask, 0}};
  bool bb[D * R * S];
  pack(R, S, C, bb);
  for (int i = 0; i < R; i++)
    for (int j = 0; j < S; j++)
      for (int l = 0; l < D; l++) {
        int p = (i * S + j) * D + l;
        bool expected = (C[i][j] >> (D - 1 - l)) & 1;
        cr_assert_eq(bb[PI(p)], expected, "C[%d][%d] = 0x%x: stream bit %d = %d, expected %d", i, j,
                     C[i][j], p, bb[PI(p)], expected);
      }
}

Test(pack, single_bit_lands_in_correct_position) {
  enum { R = 2, S = 4 };
  for (int p = 0; p < D * R * S; p++) {
    int C[R][S] = {0};
    bool bb[D * R * S];
    int entry = p / D, l = p % D;
    C[entry / S][entry % S] = 1 << (D - 1 - l);
    pack(R, S, C, bb);
    for (int t = 0; t < D * R * S; t++)
      cr_assert_eq(bb[t], t == PI(p), "bit %d set: bb[%d] = %d, expected %d", p, t, bb[t], t == PI(p));
  }
}

// Byte-level check against the reference implementation's packing: 0x7FFF followed by zeros packs to
// 0xFF 0xFE 0x00 ..., and 0x0001 to 0x00 0x02 0x00 ... (MSB-first bytes).
Test(pack, matches_reference_byte_order) {
  const struct { int value; uint8_t b0, b1; } cases[] = {{0x7FFF, 0xFF, 0xFE}, {0x0001, 0x00, 0x02},
                                                         {0x4000, 0x80, 0x00}};
  for (size_t c = 0; c < sizeof cases / sizeof cases[0]; c++) {
    int C[1][8] = {{cases[c].value}};
    bool bb[D * 8];
    pack(1, 8, C, bb);
    uint8_t b0 = 0, b1 = 0;
    for (int k = 0; k < 8; k++) {
      b0 |= bb[k] << k;
      b1 |= bb[8 + k] << k;
    }
    cr_assert(b0 == cases[c].b0 && b1 == cases[c].b1, "0x%x packed to %02x %02x, expected %02x %02x",
              cases[c].value, b0, b1, cases[c].b0, cases[c].b1);
  }
}

// --- unpack: known values ---------------------------------------------------

Test(unpack, single_entry_is_big_endian) {
  const int values[] = {0, 1, 2, 1 << (D - 1), 0xA5C3 & ((1 << D) - 1), (1 << D) - 1};
  for (size_t v = 0; v < sizeof values / sizeof values[0]; v++) {
    bool bb[D * 8] = {0};
    int C[1][8] = {{0}};
    for (int l = 0; l < D; l++) bb[PI(l)] = (values[v] >> (D - 1 - l)) & 1;
    unpack(1, 8, bb, C);
    cr_assert_eq(C[0][0], values[v], "unpacked 0x%x, expected 0x%x", C[0][0], values[v]);
  }
}

Test(unpack, single_bit_lands_in_correct_entry_and_weight) {
  enum { R = 2, S = 4 };
  for (int p = 0; p < D * R * S; p++) {
    bool bb[D * R * S] = {0};
    int C[R][S] = {0};
    bb[PI(p)] = 1;
    unpack(R, S, bb, C);
    int entry = p / D, l = p % D;
    for (int i = 0; i < R; i++)
      for (int j = 0; j < S; j++) {
        int expected = (i * S + j == entry) ? 1 << (D - 1 - l) : 0;
        cr_assert_eq(C[i][j], expected, "bit %d set: C[%d][%d] = 0x%x, expected 0x%x", p, i, j,
                     C[i][j], expected);
      }
  }
}

// unpack must overwrite the output, not accumulate into whatever was there.
Test(unpack, overwrites_existing_output) {
  enum { R = 2, S = 4 };
  bool bb[D * R * S] = {0};
  int C[R][S];
  for (int i = 0; i < R; i++)
    for (int j = 0; j < S; j++)
      C[i][j] = 0x1234;
  unpack(R, S, bb, C);
  for (int i = 0; i < R; i++)
    for (int j = 0; j < S; j++)
      cr_assert_eq(C[i][j], 0, "C[%d][%d] = 0x%x, expected 0", i, j, C[i][j]);
}

// --- round trip -------------------------------------------------------------

// Every D-bit value: 2^D cases, so this is exhaustive per entry.
Test(roundtrip, every_entry_value) {
  for (int v = 0; v < (1 << D); v++) {
    int C[1][8] = {{v}}, out[1][8] = {{0}};
    bool bb[D * 8];
    pack(1, 8, C, bb);
    unpack(1, 8, bb, out);
    cr_assert_eq(out[0][0], v, "unpack(pack(0x%x)) = 0x%x", v, out[0][0]);
  }
}

Test(roundtrip, non_square_matrices) {
  const struct { size_t r, s; } dims[] = {{1, 8}, {8, 1}, {2, 4}, {5, 8}, {mbar, nbar}, {n, nbar}, {mbar, n}};
  for (size_t d = 0; d < sizeof dims / sizeof dims[0]; d++) {
    const size_t r = dims[d].r, s = dims[d].s;
    int C[r][s], out[r][s];
    bool bb[D * r * s];
    for (size_t i = 0; i < r; i++)
      for (size_t j = 0; j < s; j++) {
        C[i][j] = (int)((i * 40503u + j * 2654435761u + 17) % (1u << D)); // irregular values
        out[i][j] = 0;
      }
    pack(r, s, C, bb);
    unpack(r, s, bb, out);
    for (size_t i = 0; i < r; i++)
      for (size_t j = 0; j < s; j++)
        cr_assert_eq(out[i][j], C[i][j], "%zux%zu: C[%zu][%zu] = 0x%x, round trip gave 0x%x", r,
                     s, i, j, C[i][j], out[i][j]);
  }
}

Test(roundtrip, unpack_then_pack) {
  enum { R = 2, S = 4 };
  bool bb[D * R * S], out[D * R * S];
  int C[R][S] = {0};
  for (int p = 0; p < D * R * S; p++) bb[p] = (p * 7 + 3) % 5 < 2; // irregular pattern
  unpack(R, S, bb, C);
  pack(R, S, C, out);
  for (int p = 0; p < D * R * S; p++)
    cr_assert_eq(out[p], bb[p], "bit %d: got %d, expected %d", p, out[p], bb[p]);
}
