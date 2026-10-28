#ifndef FRODOKEM_LINALG_H
#define FRODOKEM_LINALG_H

void matrix_add(int m, int n, int A[m][n], int B[m][n], int O[m][n]);
void matrix_mult(int m, int n, int o, int A[m][n], int B[n][o], int O[m][o]);
void transpose(int m, int n, int A[m][n], int O[n][m]);

#endif //FRODOKEM_LINALG_H
