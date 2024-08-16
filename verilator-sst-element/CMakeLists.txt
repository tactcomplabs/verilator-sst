# verilater-sst-element CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
function(print_verilator_variables VERILOG_SOURCE VERILOG_TOP VERILOG_TOP_SOURCES VERILATOR_OPTIONS VERILOG_DEVICE VERILATOR_INCLUDE VERILATORSST_SCRIPTS)
  message(STATUS "==> VERILOG_SOURCE: ${VERILOG_SOURCE}")
  message(STATUS "==> VERILOG_TOP: ${VERILOG_TOP}")
  message(STATUS "==> VERILATOR_OPTIONS: ${VERILATOR_OPTIONS}")
  message(STATUS "==> VERILOG_DEVICE: ${VERILOG_DEVICE}")
  message(STATUS "==> VERILOG_TOP_SOURCES: ${VERILOG_TOP_SOURCES}")
  message(STATUS "==> VERILATOR_INCLUDE: ${VERILATOR_INCLUDE}")
  message(STATUS "==> VERILATORSST_SCRIPTS: ${VERILATORSST_SCRIPTS}")
endfunction()

function(print_encapsulated_message MESSAGE)
  # Calculate the length of the message
  string(LENGTH "${MESSAGE}" MESSAGE_LENGTH)

  # Create a separator line of equals signs
  string(REPEAT "=" ${MESSAGE_LENGTH} SEPARATOR)

  # Print the encapsulated message
  message(STATUS "${SEPARATOR}")
  message(STATUS "${MESSAGE}")
  message(STATUS "${SEPARATOR}")
endfunction()

# -----------------------------------------------------------------
# Generate the Verilator source
# Relevant CMake variables:
# - VERILOG_SOURCE : path to the verilog source
# - VERILOG_TOP : names of the top modules
# - VERILOG_TOP_SOURCES : list of the top module files
# - VERILATOR_OPTIONS : verilator compilation options
# - VERILOG_DEVICE : device name of the target verilog module
# -----------------------------------------------------------------
# NOTE: Link handling MUST NOT be used for verilator direct
# NOTE: Cannot use clock handling AND link handling at the same time
# -----------------------------------------------------------------
# Generate the signals from the Verilator top module
# -----------------------------------------------------------------
function(generate_verilator_component VERILOG_SOURCE
                                      VERILOG_TOP
                                      VERILOG_TOP_SOURCES
                                      VERILATOR_OPTIONS
                                      VERILOG_DEVICE
                                      SST_INTERFACE
                                      CLOCK_PORT_NAME)
  # Check if INTERFACE = "Direct"
  if(SST_INTERFACE STREQUAL "Direct")
    set(ENABLE_LINK_HANDLING 0)
    set(ENABLE_CLK_HANDLING 1)
    set(VERILOG_DEVICE "${VERILOG_DEVICE}Direct")
  elseif(SST_INTERFACE STREQUAL "Links")
    set(ENABLE_LINK_HANDLING 1)
    set(ENABLE_CLK_HANDLING 0)
  else()
    message(FATAL_ERROR "Invalid SST_INTERFACE: ${SST_INTERFACE}")
  endif()
  # Print out the values of the variables
  print_verilator_variables(${VERILOG_SOURCE} ${VERILOG_TOP} ${VERILOG_TOP_SOURCES} ${VERILATOR_OPTIONS} ${VERILOG_DEVICE} ${VERILATOR_INCLUDE} ${VERILATORSST_SCRIPTS})
  find_program(CLANG_FORMAT "clang-format")
  set(VTOP "${VERILOG_SOURCE}/${VERILOG_DEVICE}/VTop.h")
  set(MESSAGE "GENERATING SST COMPONENT FOR VERILOG MODULE: ${VERILOG_TOP}")
  print_encapsulated_message(${MESSAGE})
  message(STATUS "Building verilator source...")
  execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildVerilatorSrc.sh
                            ${VERILOG_DEVICE} ${VERILOG_TOP} ${VERILOG_TOP_SOURCES} ${ENABLE_INOUT_HANDLING}
                    OUTPUT_VARIABLE VERILATOR_OUT
                    WORKING_DIRECTORY ${VERILOG_SOURCE})

  execute_process(COMMAND ${VERILATORSST_SCRIPTS}/CheckInoutPorts.sh ${VTOP}
                  TIMEOUT 1
                  RESULT_VARIABLE CHECK_INOUT_FAIL)
  if(CHECK_INOUT_FAIL)
    message(WARNING "Inout ports will not be added to the subcomponent port list, set ENABLE_INOUT_HANDLING to add these to the subcomponent")
  endif()

  message(STATUS "Building port definitions...")
  execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildPortDef.sh ${VTOP}
                    OUTPUT_VARIABLE VERILATOR_SST_PORT_DEF
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})

  message(STATUS "Building port entries...")
  execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildPortEntry.sh ${VTOP} ${VERILOG_DEVICE}
                    OUTPUT_VARIABLE VERILATOR_SST_PORT_ENTRY
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
  message(STATUS "Building port map...")
  execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildPortMap.sh ${VTOP}
                    OUTPUT_VARIABLE VERILATOR_SST_PORT_MAP
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
  message(STATUS "Building port handlers...")
  execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildPortHandlers.sh ${VTOP}
                    OUTPUT_VARIABLE VERILATOR_SST_PORT_HANDLERS
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
  message(STATUS "Building port IO...")
  execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildPortIO.sh ${VTOP}
                    OUTPUT_VARIABLE VERILATOR_SST_PORT_IO_HANDLERS
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
  message(STATUS "Building port IO implementations...")
  execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildPortIOImpls.sh ${VTOP} ${VERILOG_DEVICE}
                    OUTPUT_VARIABLE VERILATOR_SST_PORT_IO_IMPLS
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
  message(STATUS "Building port handler implementations...")
  if(ENABLE_LINK_HANDLING)
    execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildPortHandlerImpls.sh ${VTOP} ${VERILOG_DEVICE} 1 ${CLOCK_PORT_NAME}
                        OUTPUT_VARIABLE VERILATOR_SST_PORT_HANDLER_IMPLS
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
    message(STATUS "Building link definitions...")
    execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildLinkDef.sh ${VTOP}
                        OUTPUT_VARIABLE VERILATOR_SST_LINK_DEFS
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
    message(STATUS "Building link configurations...")
    execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildLinkConfig.sh ${VTOP} ${VERILOG_DEVICE}
                        OUTPUT_VARIABLE VERILATOR_SST_LINK_CONFIGS
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
  else()
    execute_process(COMMAND ${VERILATORSST_SCRIPTS}/BuildPortHandlerImpls.sh ${VTOP} ${VERILOG_DEVICE} 0
                        OUTPUT_VARIABLE VERILATOR_SST_PORT_HANDLER_IMPLS
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
    set(VERILATOR_SST_LINK_DEFS "")
    set(VERILATOR_SST_LINK_CONFIGS "")
  endif()

  if ( ENABLE_CLK_HANDLING )
    execute_process(COMMAND echo "// cycle verilator clock and apply queued writes
  std::vector<uint8_t> setHigh;
  std::vector<uint8_t> setLow;
  setLow.push_back((uint8_t)0U);
  writePort(clockPort,setLow);
  ContextP->timeInc(1);
  Top->eval();
  setHigh.push_back((uint8_t)1U);
  writePort(clockPort,setHigh);
  pollWriteQueue();
  ContextP->timeInc(1);
  Top->eval();"
  OUTPUT_VARIABLE VERILATOR_SST_CLOCK_TICK
  OUTPUT_STRIP_TRAILING_WHITESPACE )
  else()
    execute_process(COMMAND echo "// do nothing; link interface in use"
  OUTPUT_VARIABLE VERILATOR_SST_CLOCK_TICK
  OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${VERILOG_SOURCE}/${VERILOG_DEVICE})
  endif()

  # -----------------------------------------------------------------
  # Configure the verilatorSST files
  # -----------------------------------------------------------------
  configure_file(
    "${CMAKE_SOURCE_DIR}/verilator-sst-element/verilatorSSTSubcomponent.h.in"
    "${VERILOG_SOURCE}/${VERILOG_DEVICE}/verilatorSSTSubcomponent.h"
  )

  configure_file(
    "${CMAKE_SOURCE_DIR}/verilator-sst-element/verilatorSSTSubcomponent.cpp.in"
    "${VERILOG_SOURCE}/${VERILOG_DEVICE}/verilatorSSTSubcomponent.cpp"
  )

  # -----------------------------------------------------------------
  # Format the source files
  # -----------------------------------------------------------------
  if( CLANG_FORMAT )
    message(STATUS "[CLANG-FORMAT] Executing clang-format on generated source files")
    execute_process(COMMAND ${CLANG_FORMAT} -i --assume-filename=${CMAKE_CURRENT_SOURCE_DIR}/.clang-format ${VERILOG_SOURCE}/${VERILOG_DEVICE}/verilatorSSTSubcomponent.cpp)
    execute_process(COMMAND ${CLANG_FORMAT} -i --assume-filename=${CMAKE_CURRENT_SOURCE_DIR}/.clang-format ${VERILOG_SOURCE}/${VERILOG_DEVICE}/verilatorSSTSubcomponent.h)
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
    ${VERILOG_SOURCE}/${VERILOG_DEVICE}/verilatorSSTSubcomponent.cpp
    ${VERILOG_SOURCE}/${VERILOG_DEVICE}/verilatorSSTSubcomponent.h
    ${VERILATORSST_EXTERNAL_INCLUDE}/Signal.h
    ${VERILATORSST_EXTERNAL_INCLUDE}/Signal.cpp
    ${VERILATORSST_EXTERNAL_INCLUDE}/SST.h
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
  PRIVATE ${VERILOG_SOURCE}/${VERILOG_DEVICE}/libVTop.a
          ${VERILOG_SOURCE}/${VERILOG_DEVICE}/libverilated.a
)

  if(ENABLE_INOUT_HANDLING)
    add_compile_definitions(ENABLE_INOUT_HANDLING=1)
  endif()

  include_directories(${VERILATORSST_EXTERNAL_INCLUDE})
  include_directories(${VERILOG_SOURCE}/${VERILOG_DEVICE})

  # -----------------------------------------------------------------
  # Install the source
  # -----------------------------------------------------------------
  install(TARGETS ${targetName} DESTINATION ${CMAKE_SOURCE_DIR}/install)
  install(CODE "execute_process(COMMAND sst-register ${targetName} ${targetName}_LIBDIR=${CMAKE_SOURCE_DIR}/install)")


endfunction()

# -----------------------------------------------------------------
# Compile the actual component (Holds the generated subcomponent)
# -----------------------------------------------------------------
set(verilatorCompSrcs
  ${VERILATORSST_EXTERNAL_INCLUDE}/verilatorComponent.cpp
  ${VERILATORSST_EXTERNAL_INCLUDE}/verilatorComponent.h
  ${VERILATORSST_EXTERNAL_INCLUDE}/verilatorSSTAPI.h
  ${VERILATORSST_EXTERNAL_INCLUDE}/Signal.cpp
)
add_library(verilatorcomponent SHARED ${verilatorCompSrcs})
set_property(TARGET verilatorcomponent PROPERTY CXX_STANDARD 17)
target_include_directories(verilatorcomponent
                        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                        PUBLIC ${SST_INSTALL_DIR}/include
                                ${VERILATOR_INCLUDE}
                                ${VERILATOR_INCLUDE}/vltstd)

install(TARGETS verilatorcomponent DESTINATION ${CMAKE_SOURCE_DIR}/install)
install(CODE "execute_process(COMMAND sst-register verilatorcomponent verilatorcomponent_LIBDIR=${CMAKE_SOURCE_DIR}/install)")

# -- EOF
