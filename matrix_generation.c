#include "matrix_generation.h"

#include <stdint.h>

#include "fips202.h"
#include "params.h"

/**
 * Pack a bit string @param bits of length @param nbits stored one bit per element, into a byte array suitable for
 * SHAKE.
 *
 * bits[8k] becomes bit 0 of bytes[k], bits[8k+7] becomes bit 7.
 *
 * If @param nbits is not a multiple of 8, the last byte is padded with zero bits.
 *
 * Output to @param bytes .
 */
void bits_to_bytes(const size_t nbits, const uint8_t bits[nbits], uint8_t bytes[]) {
  for (size_t k = 0; k < (nbits + 7) / 8; k++) { // (nbits + 7) / 8 is equiv to ⌈nbits / 8⌉ with integer division.
    bytes[k] = 0;
  }
  for (size_t i = 0; i < nbits; i++) {
    bytes[i / 8] |= (uint8_t)((bits[i] & 1) << (i % 8));
  }
}

/**
 * The inverse of bits_to_bytes.
 */
void bytes_to_bits(const size_t nbits, const uint8_t bytes[], uint8_t bits[nbits]) {
  for (size_t i = 0; i < nbits; i++) {
    bits[i] = bytes[i / 8] >> (i % 8) & 1;
  }
}

/**
 * Algorithm 8: Frodo.Gen using SHAKE128
 *
 * Generate a pseudorandom matrix A ∈ Z_q^n×n.
 *
 * For each row index i = 0, 1, ..., n - 1, it calls SHAKE128 with a main input of seed_A, prefixed with a counter
 * (represented as a 16-bit integer in little-endian byte order), to produce a 16n-bit output string.
 *
 * It splits this output into 16-bit integers (in little-endian byte order) c_{i,j} for j = 0, 1, ..., n -1, and sets
 * A_{i,j} = c_{i,j} mod q for all j.
 *
 * @param seed_A  The random seed from which to generate the pseudorandom matrix entries.
 * @param A       The output buffer, a newly generated pseudorandom matrix.
 */
void gen_shake128(const bool seed_A[len_seed_A], int A[n][n]) {
  for (uint16_t i = 0; i < n; i++) {
    uint8_t bb[16 + len_seed_A];

    // Write i to the first 16 bits of bb
    for (int j = 0; j < 16; j++) bb[j] = i >> j & 1;

    // Write seed_A to the last len_seed_A bits of bb
    for (int j = 16; j < 16 + len_seed_A; j++) bb[j] = seed_A[j - 16];

    // Convert bb from a one-uint8_t-per-bit array to a packed byte representation, what shake128 expects
    uint8_t in[(16 + len_seed_A + 7) / 8];
    bits_to_bytes(16 + len_seed_A, bb, in);

    // Generate a pseudorandom matrix row
    uint8_t out[2 * n]; // 16n bits of output = 2n bytes
    shake128(out, sizeof out, in, sizeof in);
    uint8_t c[16 * n];
    bytes_to_bits(16 * n, out, c);

    // Each c_ij is a 16-bit little-endian integer
    for (int j = 0; j < n; j++) {
      int c_ij = 0;

      for (int k = 16 * j; k < 16 * j + 16; k++) {
        c_ij += (1 << (k - 16 * j)) * c[k];
      }

      A[i][j] = c_ij % q;
    }
  }
}