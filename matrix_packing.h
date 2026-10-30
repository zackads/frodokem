/**
 * 2.2.2 Packing matrices modulo q
 */

#ifndef MATRIX_PACKING_H
#define MATRIX_PACKING_H

#include "matrix_encoding.h"

#include <stddef.h>

void pack(size_t n_1, size_t n_2, int C[n_1][n_2], bool bb[D * n_1 * n_2]);
void unpack(size_t n_1, size_t n_2, const bool bb[D * n_1 * n_2], int C[n_1][n_2]);

#endif
