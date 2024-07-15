target_sources(${TESTS_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/csrc/testBench.cpp)
target_include_directories(${TESTS_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/csrc/)
target_compile_definitions(${TESTS_TARGET} PRIVATE -DTEST_BENCH_ADDED)
