/**
 * 2.2.1 Matrix encoding of bit strings
 */

#ifndef MATRIX_ENCODING_H
#define MATRIX_ENCODING_H

#include "params.h"

void encode(const int kk[ell], int K[mbar][nbar]);
void decode(const int K[mbar][nbar], int kk[ell]);

#endif
