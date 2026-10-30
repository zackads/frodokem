#ifndef FRODOKEM_UTILS_H
#define FRODOKEM_UTILS_H

#include <stddef.h>
#include <stdint.h>

void bits_to_bytes(size_t nbits, const bool bits[nbits], uint8_t bytes[]);
void bytes_to_bits(size_t nbits, const uint8_t bytes[], bool bits[nbits]);
void shake128_bits(size_t len_seed, const bool seed[len_seed], size_t len_out, bool out[len_out]);
void dodgy_uniform_random_bits(const size_t n, bool bits[n]);

#endif //FRODOKEM_UTILS_H
