# add csrc to target
target_sources(${TESTS_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/csrc/CounterTestBench.cpp)
target_include_directories(${TESTS_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/csrc/)
