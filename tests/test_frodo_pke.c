#include <criterion/criterion.h>
#include <string.h>

#include "../frodo_pke.h"
#include "../linalg.h"
#include "../matrix_generation.h"

// Large matrices are static to stay clear of the test runner's stack limit.
static key_pair kp, kp2;
static int A[n][n], S[n][nbar], AS[n][nbar];

// Lift a Z_q entry to its centred representative in [-q/2, q/2).
static int centre(const int x) {
  return x >= q / 2 ? x - q : x;
}

// --- key_gen ----------------------------------------------------------------

Test(key_gen, public_key_entries_are_in_Z_q) {
  kp = key_gen();
  for (int i = 0; i < n; i++)
    for (int j = 0; j < nbar; j++)
      cr_assert(0 <= kp.pk.B[i][j] && kp.pk.B[i][j] < q, "B[%d][%d] = %d", i, j, kp.pk.B[i][j]);
}

Test(key_gen, secret_key_entries_are_in_support_of_chi) {
  kp = key_gen();
  for (int i = 0; i < nbar; i++)
    for (int j = 0; j < n; j++)
      cr_assert(-s <= kp.sk[i][j] && kp.sk[i][j] <= s, "S^T[%d][%d] = %d", i, j, kp.sk[i][j]);
}

// B - AS = E must be a small error matrix, with A regenerated from seed_A.
Test(key_gen, public_key_is_an_LWE_sample) {
  kp = key_gen();
  gen_shake128(kp.pk.seed_A, A);
  transpose(nbar, n, kp.sk, S);
  matrix_mult(n, n, nbar, A, S, AS);
  for (int i = 0; i < n; i++)
    for (int j = 0; j < nbar; j++) {
      const int e = centre((kp.pk.B[i][j] - AS[i][j]) & (q - 1));
      cr_assert(-s <= e && e <= s, "E[%d][%d] = %d", i, j, e);
    }
}

Test(key_gen, seeds_are_random) {
  kp = key_gen();
  kp2 = key_gen();
  cr_assert_neq(memcmp(kp.pk.seed_A, kp2.pk.seed_A, sizeof kp.pk.seed_A), 0, "seed_A repeated across calls");
  cr_assert_neq(memcmp(kp.sk, kp2.sk, sizeof kp.sk), 0, "S^T repeated across calls");
}
