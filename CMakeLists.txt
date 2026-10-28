cmake_minimum_required(VERSION 4.2)
project(frodokem C)

set(CMAKE_C_STANDARD 23)

add_executable(frodokem main.c
        matrix_encoding.c
        matrix_packing.c
        sampling.c
        sampling.h
        fips202.c
        matrix_generation.c
        matrix_generation.h
        frodo_pke.c
        frodo_pke.h
        params.h
)

# Tests (Criterion: brew install criterion)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED IMPORTED_TARGET criterion)

enable_testing()

add_executable(test_matrix_encoding tests/test_matrix_encoding.c
        matrix_encoding.c)
target_link_libraries(test_matrix_encoding PRIVATE PkgConfig::CRITERION)
# UBSan only: ASan hangs under Criterion's forked test runner on macOS.
target_compile_options(test_matrix_encoding PRIVATE -fsanitize=undefined)
target_link_options(test_matrix_encoding PRIVATE -fsanitize=undefined)
add_test(NAME matrix_encoding COMMAND test_matrix_encoding --timeout 10)

add_executable(test_matrix_packing tests/test_matrix_packing.c
        matrix_packing.c)
target_link_libraries(test_matrix_packing PRIVATE PkgConfig::CRITERION)
target_compile_options(test_matrix_packing PRIVATE -fsanitize=undefined)
target_link_options(test_matrix_packing PRIVATE -fsanitize=undefined)
add_test(NAME matrix_packing COMMAND test_matrix_packing --timeout 10)
