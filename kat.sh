#!/usr/bin/env bash
# Checks the CLI against the Known Answer Tests in PQCkemKAT_19888_shake.rsp.
#
# The CLI draws its randomness from rand() rather than the KAT's seeded DRBG, so keygen and encaps can't reproduce
# the expected pk, sk and ct. Instead, for each KAT this script:
#   1. decapsulates the KAT ciphertext with the KAT secret key and asserts the result equals the KAT shared secret;
#   2. encapsulates against the KAT public key and asserts decapsulating that ciphertext with the KAT secret key
#      recovers the same shared secret.
#
# Usage: ./kat.sh [rsp-file]
set -euo pipefail

cd "$(dirname "$0")"
rsp=${1:-PQCkemKAT_19888_shake.rsp}

cmake -S . -B build >/dev/null
cmake --build build --target frodokem >/dev/null
bin=./build/frodokem

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

lower() { tr '[:upper:]' '[:lower:]' <<<"$1"; }

passed=0
failed=0

fail() {
  echo "FAIL count=$count: $1" >&2
  failed=$((failed + 1))
}

run_kat() {
  printf '%s\n' "$pk" >"$tmp/pk"
  printf '%s\n' "$sk" >"$tmp/sk"
  printf '%s\n' "$ct" >"$tmp/ct"
  local expected got encaps_out encaps_ss
  expected=$(lower "$ss")

  if ! got=$("$bin" decaps "$tmp/sk" "$tmp/ct"); then
    fail "decaps exited with an error"
    return
  fi
  if [[ $got != "$expected" ]]; then
    fail "decaps shared secret mismatch"$'\n'"  expected: $expected"$'\n'"  got:      $got"
    return
  fi

  if ! encaps_out=$("$bin" encaps "$tmp/pk" "$tmp/ct2"); then
    fail "encaps exited with an error"
    return
  fi
  encaps_ss=${encaps_out#The shared secret is: }
  if ! got=$("$bin" decaps "$tmp/sk" "$tmp/ct2"); then
    fail "decaps of fresh ciphertext exited with an error"
    return
  fi
  if [[ $got != "$encaps_ss" ]]; then
    fail "encaps/decaps round trip mismatch"$'\n'"  encaps: $encaps_ss"$'\n'"  decaps: $got"
    return
  fi

  echo "ok   count=$count"
  passed=$((passed + 1))
}

count='' pk='' sk='' ct='' ss=''
while IFS=' =' read -r key value; do
  case $key in
    count) count=$value ;;
    pk) pk=$value ;;
    sk) sk=$value ;;
    ct) ct=$value ;;
    ss)
      ss=$value
      run_kat
      ;;
  esac
done <"$rsp"

echo "$passed passed, $failed failed"
if ((passed + failed == 0)); then
  echo "no KATs found in $rsp" >&2
  exit 1
fi
((failed == 0))
