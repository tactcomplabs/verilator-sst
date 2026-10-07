# examples/ CMakeLists.txt
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#
# Example SST components that *use* verilated models but are not part of the
# test harness. These depend on the verilatorsst<Device> subcomponent
# libraries generated under test/, so they're only added when testing is on.

add_subdirectory(accum-driver)
add_subdirectory(accum-driver-direct)
add_subdirectory(accum-offload)

# EOF
