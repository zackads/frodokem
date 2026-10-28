#ifndef FRODOKEM_SAMPLING_H
#define FRODOKEM_SAMPLING_H

#include "params.h"

int sample(const bool *rr);
void sample_matrix(size_t n_1, size_t n_2, const bool rr[n_1 * n_2][len_chi], int E[n_1][n_2]);

#endif //FRODOKEM_SAMPLING_H