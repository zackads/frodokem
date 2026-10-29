#include "linalg.h"
#include "params.h"

/**
 * Reduce x modulo q into [0, q). Since q is a power of two, this is a mask of the low D bits, which is also correct
 * for negative x in two's complement.
 */
static int mod_q(const int x) {
  return x & (q - 1);
}

void matrix_mult(const int m, const int n, const int o, int A[m][n], int B[n][o], int O[m][o]) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < o; j++) {
      O[i][j] = 0;
      for (int k = 0; k < n; k++) {
        O[i][j] += A[i][k] * B[k][j];
      }
      O[i][j] = mod_q(O[i][j]);
    }
  }
}

void matrix_add(const int m, const int n, int A[m][n], int B[m][n], int O[m][n]) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      O[i][j] = mod_q(A[i][j] + B[i][j]);
    }
  }
}

void transpose(const int m, const int n, int A[m][n], int O[n][m]) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      O[j][i] = A[i][j];
    }
  }
}

void matrix_scale(const int m, const int n, const int lambda, int A[m][n], int O[m][n]) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      O[i][j] = mod_q(lambda * A[i][j]);
    }
  }
}
