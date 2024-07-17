
# built test library target
set(TESTS_TARGET verilatorssttests)
add_library(${TESTS_TARGET} SHARED "")

# incldue SST and Subcomponent base header files
target_include_directories(${TESTS_TARGET}
    PRIVATE ${VERILATORSST_EXTERNAL_INCLUDE} #move base out of configured subcomponent header file
            ${VERILATORSST_EXTERNAL_INCLUDE}/${VERILOG_DEVICE}
    PUBLIC ${SST_INSTALL_DIR}/include
            ${VERILATOR_INCLUDE}
            ${VERILATOR_INCLUDE}/vltstd)

# compile/link with SST flags
target_compile_options(${TESTS_TARGET} PRIVATE ${SST_CXXFLAGS_LIST})
target_link_options(${TESTS_TARGET} PRIVATE ${SST_LDFLAGS_LIST})

# register library with SST
install(TARGETS ${TESTS_TARGET} DESTINATION ${CMAKE_SOURCE_DIR}/install)
install(CODE "execute_process(COMMAND sst-register ${TESTS_TARGET} ${TESTS_TARGET}_LIBDIR=${CMAKE_SOURCE_DIR}/install)")

# add test harness
add_subdirectory(harness)

# add tests directories
add_subdirectory(api_tests)
add_subdirectory(functional_tests)
add_subdirectory(integration_tests)
