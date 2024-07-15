set(TESTS_TARGET verilatorssttests)
add_library(${TESTS_TARGET} SHARED "")
add_subdirectory(harness)
add_subdirectory(api_tests)
