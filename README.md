# frodokem

A C implementation of FrodoKEM, following the
[2021-06-04 specification](FrodoKEM-specification-20210604.pdf).

## Prerequisites

- A C compiler with C23 support (recent Clang or GCC)
- CMake 4.2 or newer
- [Criterion](https://github.com/Snaipe/Criterion) and `pkg-config`, for the tests

On macOS:

```sh
brew install cmake pkg-config criterion
```

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Run

```sh
./build/frodokem
```

## Test

```sh
ctest --test-dir build --output-on-failure
```

To run a single test suite directly (Criterion options such as `--verbose` work here):

```sh
./build/test_matrix_encoding
./build/test_matrix_packing
```

The tests are built with UndefinedBehaviorSanitizer. AddressSanitizer is
deliberately left out because it hangs under Criterion's forked test runner on
macOS.
