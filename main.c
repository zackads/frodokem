#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "frodo_kem.h"
#include "utils.h"

// Serialised sizes in bytes. The secret key is s || seed_A || b || S^T || pkh, with S^T as 16-bit little-endian
// integers, matching the reference implementation's 19888-byte format.
constexpr size_t pk_bytes = len_pk / 8;
constexpr size_t ct_bytes = (len_c1 + len_c2) / 8;
constexpr size_t ss_bytes = len_ss / 8;
constexpr size_t sk_bytes = len_s / 8 + pk_bytes + 2 * nbar * n + len_pkh / 8;

static void usage(const char *prog) {
  fprintf(stderr,
          "Usage:\n"
          "  %s keygen <pk-file> <sk-file>   Generate a key pair\n"
          "  %s encaps <pk-file> <ct-file>   Write a ciphertext, print the shared secret\n"
          "  %s decaps <sk-file> <ct-file>   Print the shared secret recovered from a ciphertext\n",
          prog, prog, prog);
}

static void fprint_hex(FILE *f, const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    fprintf(f, "%02x", buf[i]);
  }
  fprintf(f, "\n");
}

// Keys and ciphertexts are stored as a single line of lowercase hex so they can be read and copied as plain text.
static void write_hex_file(const char *path, const uint8_t *buf, size_t len) {
  FILE *f = fopen(path, "w");
  if (f == nullptr) {
    perror(path);
    exit(EXIT_FAILURE);
  }
  fprint_hex(f, buf, len);
  if (fclose(f) != 0) {
    perror(path);
    exit(EXIT_FAILURE);
  }
}

static void read_hex_file(const char *path, uint8_t *buf, size_t len) {
  FILE *f = fopen(path, "r");
  if (f == nullptr) {
    perror(path);
    exit(EXIT_FAILURE);
  }
  char c;
  for (size_t i = 0; i < len; i++) {
    if (fscanf(f, "%2hhx", &buf[i]) != 1) {
      fprintf(stderr, "%s: expected %zu hex-encoded bytes\n", path, len);
      exit(EXIT_FAILURE);
    }
  }
  if (fscanf(f, " %c", &c) != EOF) {
    fprintf(stderr, "%s: expected %zu hex-encoded bytes\n", path, len);
    exit(EXIT_FAILURE);
  }
  fclose(f);
}

static void print_hex(const bool bits[len_ss]) {
  uint8_t bytes[ss_bytes];
  bits_to_bytes(len_ss, bits, bytes);
  fprint_hex(stdout, bytes, ss_bytes);
}

static int cmd_keygen(const char *pk_path, const char *sk_path) {
  const kem_keypair kp = keygen();

  uint8_t pk[pk_bytes];
  bits_to_bytes(len_pk, kp.pk, pk);
  write_hex_file(pk_path, pk, sizeof pk);

  uint8_t sk[sk_bytes];
  uint8_t *p = sk;
  bits_to_bytes(len_s, kp.sk.s, p);
  p += len_s / 8;
  bits_to_bytes(len_seed_A, kp.sk.seed_A, p);
  p += len_seed_A / 8;
  bits_to_bytes(len_b, kp.sk.b, p);
  p += len_b / 8;
  for (int i = 0; i < nbar; i++) {
    for (int j = 0; j < n; j++) {
      const uint16_t x = (uint16_t)kp.sk.S_T[i][j];
      *p++ = x & 0xFF;
      *p++ = x >> 8;
    }
  }
  bits_to_bytes(len_pkh, kp.sk.pkh, p);
  write_hex_file(sk_path, sk, sizeof sk);
  return EXIT_SUCCESS;
}

static int cmd_encaps(const char *pk_path, const char *ct_path) {
  uint8_t pk_in[pk_bytes];
  read_hex_file(pk_path, pk_in, sizeof pk_in);
  bool pk[len_pk];
  bytes_to_bits(len_pk, pk_in, pk);

  const kem_encaps_result res = encaps(pk);

  uint8_t ct[ct_bytes];
  bits_to_bytes(len_c1 + len_c2, res.ciphertext, ct);
  write_hex_file(ct_path, ct, sizeof ct);
  printf("The shared secret is: ");
  print_hex(res.ss);
  return EXIT_SUCCESS;
}

static int cmd_decaps(const char *sk_path, const char *ct_path) {
  uint8_t sk_in[sk_bytes];
  read_hex_file(sk_path, sk_in, sizeof sk_in);

  // struct kem_sk has const members, so fill a heap allocation through non-const pointers.
  struct kem_sk *sk = malloc(sizeof *sk);
  if (sk == nullptr) {
    perror("malloc");
    return EXIT_FAILURE;
  }
  const uint8_t *p = sk_in;
  bytes_to_bits(len_s, p, (bool *)sk->s);
  p += len_s / 8;
  bytes_to_bits(len_seed_A, p, (bool *)sk->seed_A);
  p += len_seed_A / 8;
  bytes_to_bits(len_b, p, (bool *)sk->b);
  p += len_b / 8;
  int (*S_T)[n] = (int (*)[n])sk->S_T;
  for (int i = 0; i < nbar; i++) {
    for (int j = 0; j < n; j++) {
      S_T[i][j] = (int16_t)(p[0] | p[1] << 8);
      p += 2;
    }
  }
  bytes_to_bits(len_pkh, p, (bool *)sk->pkh);

  uint8_t ct_in[ct_bytes];
  read_hex_file(ct_path, ct_in, sizeof ct_in);
  bool ct[len_c1 + len_c2];
  bytes_to_bits(len_c1 + len_c2, ct_in, ct);

  bool ss[len_ss];
  decaps(ct, sk, ss);
  free(sk);
  print_hex(ss);
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  // NOTE: rand() is not a secure source of randomness; this only stops every run producing identical keys.
  srand((unsigned)time(nullptr) ^ (unsigned)getpid());

  if (strcmp(argv[1], "keygen") == 0) return cmd_keygen(argv[2], argv[3]);
  if (strcmp(argv[1], "encaps") == 0) return cmd_encaps(argv[2], argv[3]);
  if (strcmp(argv[1], "decaps") == 0) return cmd_decaps(argv[2], argv[3]);

  usage(argv[0]);
  return EXIT_FAILURE;
}
