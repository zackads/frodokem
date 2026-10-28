#include "sampling.h"
#include "params.h"

#include <stddef.h>

/**
 * Sampling from the error distribution
 *
 * The error distribution chi used in FrodoKem is a discrete, symmetric distribution on the integers, centred on zero
 * and with small support, which approximates a rounded continuous Gaussian distribution.
 */

/**
 * Sample from the distribution chi via inversion sampling
 *
 * @param rr
 * @param T_chi
 * @return
 */
int sample(const int rr[len_chi], const int T_chi[s]) {
  // Line 1
  int t = 0;
  for (int i = 0; i < len_chi; i++) {
    t += rr[i] << (i-1);
  }
  int e = 0;

  for (int z = 0; z < s; z++) {
    if (t > T_chi[z]) { // TODO: This comparison is not constant time?
      e++;
    }
  }

  if (rr[0] == 1) {
    e = -e;
  }
  return e;
}

void sample_matrix(
  const size_t n_1,
  const size_t n_2,
  const int rr[n_1 * n_2][len_chi],
  const int T_chi[s],
  int E[n_1][n_2]) {
  for (int i = 0; i < n_1; i++) {
    for (int j = 0; j < n_2; j++) {
      E[i][j] = sample(rr[i * n_2 + j], T_chi);
    }
  }
}