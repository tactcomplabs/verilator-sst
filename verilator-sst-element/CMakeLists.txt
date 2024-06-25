# verilater-sst-element CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

# -----------------------------------------------------------------
# Generate the Verilator source
# Relevant CMake variables:
# - VERILOG_SOURCE : path to the verilog source
# - VERILOG_TOP : names of the top modules
# - VERILOG_TOP_SOURCES : list of the top module files
# - VERILATOR_OPTIONS : verilator compilation options
# - VERILOG_DEVICE : device name of the target verilog module
# -----------------------------------------------------------------

# -----------------------------------------------------------------
# Generate the signals from the Verilator top
# -----------------------------------------------------------------
set(VTOP "${CMAKE_CURRENT_SOURCE_DIR}/${VERILOG_DEVICE}/VTop.h")
message(STATUS "Building verilator source...")
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/BuildVerilatorSrc.sh
                        ${VERILOG_DEVICE} ${VERILOG_TOP} ${VERILOG_TOP_SOURCES}
                OUTPUT_VARIABLE VERILATOR_OUT
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Building port definitions...")
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/BuildPortDef.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_DEF
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port entries...")
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/BuildPortEntry.sh ${VTOP} ${VERILOG_DEVICE}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_ENTRY
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port map...")
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/BuildPortMap.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_MAP
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port handlers...")
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/BuildPortHandlers.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_HANDLERS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port IO...")
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/BuildPortIO.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_IO_HANDLERS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port IO implementations...")
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/BuildPortIOImpls.sh ${VTOP} ${VERILOG_DEVICE}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_IO_IMPLS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port handler implementations...")
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/BuildPortHandlerImpls.sh ${VTOP} ${VERILOG_DEVICE}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_HANDLER_IMPLS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# -----------------------------------------------------------------
# Configure the verilatorSST files
# -----------------------------------------------------------------
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSSTSubcomponent.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSSTSubcomponent.h"
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSSTSubcomponent.cpp.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSSTSubcomponent.cpp"
)

# -----------------------------------------------------------------
# Format the source files
# -----------------------------------------------------------------
find_program(CLANG_FORMAT "clang-format")
if( CLANG_FORMAT )
  message(STATUS "[CLANG-FORMAT] Executing clang-format on generated source files")
  execute_process(COMMAND ${CLANG_FORMAT} -i --assume-filename=${CMAKE_CURRENT_SOURCE_DIR}/.clang-format ${CMAKE_CURRENT_SOURCE_DIR}/verilatorSSTSubcomponent.cpp)
  execute_process(COMMAND ${CLANG_FORMAT} -i --assume-filename=${CMAKE_CURRENT_SOURCE_DIR}/.clang-format ${CMAKE_CURRENT_SOURCE_DIR}/verilatorSSTSubcomponent.h)
endif()


# -----------------------------------------------------------------
# Discover all the source files
# -----------------------------------------------------------------
MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.h)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES(header_dir_list)
list(LENGTH header_dir_list header_dir_list_count)
message(STATUS "[INFO] Found ${header_dir_list_count} header directories.")

MACRO(CPP_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.cpp)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

CPP_DIRECTORIES(cpp_dir_list)
list(LENGTH cpp_dir_list cpp_dir_list_count)
message(STATUS "[INFO] Found ${cpp_dir_list_count} cpp directories.")

file(GLOB_RECURSE VSRCS *.cpp)
file(GLOB_RECURSE VHDRS *.h)

# -----------------------------------------------------------------
# Build the entire source
# -----------------------------------------------------------------
set(targetName "verilatorsst${VERILOG_DEVICE}")
set(verilatorSSTSrcs
  verilatorSSTSubcomponent.cpp
  verilatorSSTSubcomponent.h
  Signal.cpp
  Signal.h
)

add_library(${targetName} SHARED ${verilatorSSTSrcs})
set_property(TARGET ${targetName} PROPERTY CXX_STANDARD 17)
target_include_directories(${targetName}
                          PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                  ${CMAKE_CURRENT_SOURCE_DIR}/${VERILOG_DEVICE}
                          PUBLIC ${SST_INSTALL_DIR}/include
                                 ${VERILATOR_INCLUDE}
                                 ${VERILATOR_INCLUDE}/vltstd)
target_link_libraries(${targetName}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${VERILOG_DEVICE}/libVTop.a
          ${CMAKE_CURRENT_SOURCE_DIR}/${VERILOG_DEVICE}/libverilated.a
)

set(verilatorCompSrcs
  verilatorComponent.cpp
  verilatorComponent.h
)
add_library(verilatorcomponent SHARED ${verilatorCompSrcs})
set_property(TARGET verilatorcomponent PROPERTY CXX_STANDARD 17)
target_include_directories(verilatorcomponent
                          PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                  ${CMAKE_CURRENT_SOURCE_DIR}/${VERILOG_DEVICE}
                          PUBLIC ${SST_INSTALL_DIR}/include
                                 ${VERILATOR_INCLUDE}
                                 ${VERILATOR_INCLUDE}/vltstd)

# -----------------------------------------------------------------
# Install the source
# -----------------------------------------------------------------
install(TARGETS ${targetName} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register ${targetName} ${targetName}_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

install(TARGETS verilatorcomponent DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register verilatorcomponent verilatorcomponent_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

#if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#  install(CODE "execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/test_undefined_symbols.sh ${CMAKE_CURRENT_SOURCE_DIR}/librevcpu.dylib)")
#else()
#  install(CODE "execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/test_undefined_symbols.sh ${CMAKE_CURRENT_SOURCE_DIR}/librevcpu.so)")
#endif()


# -- EOF
