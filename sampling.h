

#ifndef FRODOKEM_SAMPLING_H
#define FRODOKEM_SAMPLING_H

#include "params.h"

#endif //FRODOKEM_SAMPLING_H
int sample(const int *rr, const int *T_chi);

void sample_matrix(size_t n_1, size_t n_2, const int rr[n_1 * n_2][len_chi], const int T_chi[s], int E[n_1][n_2]);