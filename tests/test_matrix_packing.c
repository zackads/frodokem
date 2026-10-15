#include <criterion/criterion.h>

#include "../matrix_packing.h"

// --- pack: known values -----------------------------------------------------

// Entries are written as D-bit big-endian strings, row by row.
Test(pack, single_entry_is_big_endian) {
  const int values[] = {0, 1, 2, 0x8000, 0xA5C3, (1 << D) - 1};
  for (size_t v = 0; v < sizeof values / sizeof values[0]; v++) {
    int C[1][1] = {{values[v]}}, bb[D];
    for (int p = 0; p < D; p++) bb[p] = -1;
    pack(1, 1, C, bb);
    for (int l = 0; l < D; l++) {
      int expected = (values[v] >> (D - 1 - l)) & 1;
      cr_assert_eq(bb[l], expected, "C = 0x%x: bit %d = %d, expected %d", values[v], l, bb[l],
                   expected);
    }
  }
}

Test(pack, entries_are_concatenated_row_by_row) {
  enum { R = 2, S = 3 };
  int C[R][S] = {{0x0001, 0x8000, 0x00FF}, {0xFF00, 0x1234, 0xFFFF}}, bb[D * R * S];
  pack(R, S, C, bb);
  for (int i = 0; i < R; i++)
    for (int j = 0; j < S; j++)
      for (int l = 0; l < D; l++) {
        int p = (i * S + j) * D + l, expected = (C[i][j] >> (D - 1 - l)) & 1;
        cr_assert_eq(bb[p], expected, "C[%d][%d] = 0x%x: bb[%d] = %d, expected %d", i, j,
                     C[i][j], p, bb[p], expected);
      }
}

Test(pack, single_bit_lands_in_correct_position) {
  enum { R = 2, S = 3 };
  for (int p = 0; p < D * R * S; p++) {
    int C[R][S] = {0}, bb[D * R * S];
    int entry = p / D, l = p % D;
    C[entry / S][entry % S] = 1 << (D - 1 - l);
    pack(R, S, C, bb);
    for (int t = 0; t < D * R * S; t++)
      cr_assert_eq(bb[t], t == p, "bit %d set: bb[%d] = %d, expected %d", p, t, bb[t], t == p);
  }
}

// --- unpack: known values ---------------------------------------------------

Test(unpack, single_entry_is_big_endian) {
  const int values[] = {0, 1, 2, 0x8000, 0xA5C3, (1 << D) - 1};
  for (size_t v = 0; v < sizeof values / sizeof values[0]; v++) {
    int bb[D], C[1][1] = {{0}};
    for (int l = 0; l < D; l++) bb[l] = (values[v] >> (D - 1 - l)) & 1;
    unpack(1, 1, bb, C);
    cr_assert_eq(C[0][0], values[v], "unpacked 0x%x, expected 0x%x", C[0][0], values[v]);
  }
}

Test(unpack, single_bit_lands_in_correct_entry_and_weight) {
  enum { R = 2, S = 3 };
  for (int p = 0; p < D * R * S; p++) {
    int bb[D * R * S] = {0}, C[R][S] = {0};
    bb[p] = 1;
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
  enum { R = 2, S = 2 };
  int bb[D * R * S] = {0}, C[R][S];
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
    int C[1][1] = {{v}}, bb[D], out[1][1] = {{0}};
    pack(1, 1, C, bb);
    unpack(1, 1, bb, out);
    cr_assert_eq(out[0][0], v, "unpack(pack(0x%x)) = 0x%x", v, out[0][0]);
  }
}

Test(roundtrip, non_square_matrices) {
  const struct { size_t r, s; } dims[] = {{1, 1}, {1, 5}, {5, 1}, {2, 3}, {mbar, nbar}, {n, nbar}};
  for (size_t d = 0; d < sizeof dims / sizeof dims[0]; d++) {
    const size_t r = dims[d].r, s = dims[d].s;
    int C[r][s], out[r][s], bb[D * r * s];
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
  enum { R = 2, S = 3 };
  int bb[D * R * S], C[R][S] = {0}, out[D * R * S];
  for (int p = 0; p < D * R * S; p++) bb[p] = (p * 7 + 3) % 5 < 2; // irregular pattern
  unpack(R, S, bb, C);
  pack(R, S, C, out);
  for (int p = 0; p < D * R * S; p++)
    cr_assert_eq(out[p], bb[p], "bit %d: got %d, expected %d", p, out[p], bb[p]);
}
