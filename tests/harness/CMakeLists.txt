target_sources(${TESTS_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/testHarness.cpp)
target_include_directories(${TESTS_TARGET}
                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                        ${VERILATORSST_EXTERNAL_INCLUDE}
                        ${VERILATORSST_EXTERNAL_INCLUDE}/${VERILOG_DEVICE}
                PUBLIC ${SST_INSTALL_DIR}/include
                       ${VERILATOR_INCLUDE}
                       ${VERILATOR_INCLUDE}/vltstd)

target_compile_options(${TESTS_TARGET} PRIVATE ${SST_CXXFLAGS_LIST})
target_link_options(${TESTS_TARGET} PRIVATE ${SST_LDFLAGS_LIST})

install(TARGETS ${TESTS_TARGET} DESTINATION ${CMAKE_SOURCE_DIR}/install)
install(CODE "execute_process(COMMAND sst-register ${TESTS_TARGET} ${TESTS_TARGET}_LIBDIR=${CMAKE_SOURCE_DIR}/install)")
