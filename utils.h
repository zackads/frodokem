#ifndef FRODOKEM_UTILS_H
#define FRODOKEM_UTILS_H

#include <stddef.h>
#include <stdint.h>

void bits_to_bytes(size_t nbits, const bool bits[nbits], uint8_t bytes[]);
void bytes_to_bits(size_t nbits, const uint8_t bytes[], bool bits[nbits]);
void shake128_bits(size_t prefix_len, uint32_t prefix, size_t seed_len, const bool seed[seed_len],
                   size_t out_len, bool out[out_len]);

#endif //FRODOKEM_UTILS_H
