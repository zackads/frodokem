#include <stdint.h>
#include "utils.h"

#include "fips202.h"

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
void bits_to_bytes(const size_t nbits, const bool bits[nbits], uint8_t bytes[]) {
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
void bytes_to_bits(const size_t nbits, const uint8_t bytes[], bool bits[nbits]) {
  for (size_t i = 0; i < nbits; i++) {
    bits[i] = bytes[i / 8] >> (i % 8) & 1;
  }
}

/**
 * Compute SHAKE128(prefix || seed, out_len) on one-bool-per-bit strings.
 *
 * @param prefix is written as a @param prefix_len -bit little-endian integer (prefix_len <= 32), followed by the
 * @param seed_len bits of @param seed .
 *
 * Output the @param out_len pseudorandom bits to @param out .
 */
void shake128_bits(const size_t prefix_len, const uint32_t prefix, const size_t seed_len, const bool seed[seed_len],
                   const size_t out_len, bool out[out_len]) {
  bool bb[prefix_len + seed_len];
  for (size_t i = 0; i < prefix_len; i++) bb[i] = (prefix >> i) & 1;
  for (size_t i = prefix_len; i < prefix_len + seed_len; i++) bb[i] = seed[i - prefix_len];

  // Convert bb from a one-bool-per-bit array to a packed byte representation, what shake128 expects
  uint8_t in[(prefix_len + seed_len + 7) / 8];
  bits_to_bytes(prefix_len + seed_len, bb, in);

  uint8_t bytes_out[(out_len + 7) / 8];
  shake128(bytes_out, sizeof bytes_out, in, sizeof in);

  bytes_to_bits(out_len, bytes_out, out);
}
