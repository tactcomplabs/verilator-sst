# Copyright (C) 2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com

# Prevent in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD IN-TREE!")
endif()

# Minimum required version of CMake and project information
cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0074 NEW)

project(verilatorsst)

#------------------------------------------------------------------
# SST SETUP
#------------------------------------------------------------------
find_program(SST sst)
find_program(SST_CONFIG sst-config)
if(NOT SST OR NOT SST_CONFIG)
  message(FATAL_ERROR "No SST binary or sst-config binary found in path")
endif()
execute_process(COMMAND sst-config --prefix
                OUTPUT_VARIABLE SST_INSTALL_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND sst-config --CXX
                OUTPUT_VARIABLE CXX
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND sst-config --ELEMENT_CXXFLAGS
                OUTPUT_VARIABLE SST_CXXFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CXXFLAGS "${SST_CXXFLAGS}  -fno-stack-protector")
execute_process(COMMAND sst-config --ELEMENT_LDFLAGS
                OUTPUT_VARIABLE SST_LDFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "[SST] SST_INSTALL_DIR=${SST_INSTALL_DIR}")
message(STATUS "[SST] SST CXX=${CXX}")
message(STATUS "[SST] SST_CXXFLAGS=${SST_CXXFLAGS}")
message(STATUS "[SST] SST_LDFLAGS=${SST_LDFLAGS}")

set(CXXFLAGS "${SST_CXXFLAGS}")
set(LDFLAGS "${SST_LDFLAGS}")

#------------------------------------------------------------------
# VERILATOR SETUP
#------------------------------------------------------------------
find_package(verilator HINTS $ENV{VERILATOR_ROOT} ${VERILATOR_ROOT})
if (NOT verilator_FOUND)
	message(FATAL_ERROR "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()
if(NOT VERILATOR_INCLUDE)
	set(VERILATOR_INCLUDE "${VERILATOR_ROOT}/include")
endif()

message(STATUS "[VERILATOR] VERILATOR_INCLUDE=${VERILATOR_INCLUDE}")

#------------------------------------------------------------------
# VERILATORSSTF FUNCTION
#------------------------------------------------------------------

function(verilatorsstf TARGET)
	cmake_parse_arguments(VERILATORSSTF "" "VSRC_DIR;CSRC_DIR;TOP_MODULE" "VERILATOR_ARGS" ${ARGN})

	# check required arguments
	if (NOT VERILATORSSTF_VSRC_DIR)
		message(FATAL_ERROR "VSRC_DIR not defined")
	endif()
	if (NOT VERILATORSSTF_CSRC_DIR)
		message(FATAL_ERROR "CSRC_DIR not defined")
	endif()
	if (NOT VERILATORSSTF_TOP_MODULE)
		message(FATAL_ERROR "TOP_MODULE not defined")
	endif()

	# verilate verilog files
	set(VTOP_LIB "vtop")
    set(MDIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${VTOP_LIB}.dir)

    find_program(VERILATOR verilator)
    set(VERILATOR_COMMAND ${VERILATOR}
        --threads 1 --cc --build --vpi --public-flat-rw
        -CFLAGS \"-fPIC -std=c++17\" -LDFLAGS \"${SST_LDFLAGS}\"
        -Wall -Mdir ${MDIR} --prefix VTop
        -I${VERILATORSSTF_VSRC_DIR} ${VERILATORSSTF_VERILATOR_ARGS} ${VERILATORSSTF_TOP_MODULE})

    add_custom_target(${VTOP_LIB}
        COMMAND ${VERILATOR_COMMAND}
        COMMENT "Verilating verilog files")
    add_dependencies(${TARGET} ${VTOP_LIB})
        
    # add verilator output files to target dependencies
	target_include_directories(${TARGET}
		PRIVATE ${MDIR})
    target_link_libraries(${TARGET}
        PRIVATE ${MDIR}/libverilated.a
                ${MDIR}/libVTop.a)

    # add verilator includes to target
    target_include_directories(${TARGET}
        PUBLIC  ${VERILATOR_INCLUDE}
                ${VERILATOR_INCLUDE}/vltstd)

    # add verilatorsst files to target
	target_sources(${TARGET}
        PRIVATE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/verilatorsst/Signal.cc)
    target_include_directories(${TARGET}
        PRIVATE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/verilatorsst)

	# add user files to target dependencies
	file(GLOB USER_SOURCE_FILES
		"${VERILATORSSTF_CSRC_DIR}/*.cc"
		"${VERILATORSSTF_CSRC_DIR}/*.cpp")

	target_sources(${TARGET}
		PUBLIC ${USER_SOURCE_FILES})
	target_include_directories(${TARGET}
		PUBLIC ${VERILATORSSTF_CSRC_DIR})

    # add sst includes to target
    target_include_directories(${TARGET}
        PUBLIC ${SST_INSTALL_DIR}/include)

    # # # configure subcomponent
    # set(SUBCOMPONENT ${CMAKE_CURRENT_BINARY_DIR}/verilatorSSTSubcomponent.cpp ${CMAKE_CURRENT_BINARY_DIR}/verilatorSSTSubcomponent.h)
    # add_custom_command(OUTPUT ${SUBCOMPONENT}
    #     COMMAND ${CMAKE_COMMAND} -DVTOP_HEADER=${MDIR}/VTop.h -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/subcomponent.cmake
    #     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    #     DEPENDS ${VTOP_LIB}
    #     COMMENT "Configuring subcomponents")

    # # # add subcomponent files to target
    # target_sources(${TARGET}
    #     PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/verilatorSSTSubcomponent.cpp)
    # target_include_directories(${TARGET}
    #     PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    # add SST flags
    separate_arguments(SST_CXXFLAGS_LIST UNIX_COMMAND ${SST_CXXFLAGS})
    target_compile_options(${TARGET} PRIVATE ${SST_CXXFLAGS_LIST})
    separate_arguments(SST_LDFLAGS_LIST UNIX_COMMAND ${SST_LDFLAGS})
    target_link_options(${TARGET} PRIVATE ${SST_LDFLAGS_LIST})
    set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "verilatorsst")

    # set install location and comamnds
    install(TARGETS ${TARGET} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
    install(CODE "execute_process(COMMAND sst-register ${TARGET} ${TARGET}_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
endfunction()
