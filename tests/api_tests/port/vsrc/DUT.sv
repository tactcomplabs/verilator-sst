// tests/api_tests/port/vsrc/DUT.sv
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details

module DUT(
    input input_port,
    output output_port,
    inout inout_port,

    input [8887:0] wide_port,
    output array_port [0:9998],
    input [8887:0] wide_array_port [0:9998],

    output logic logic_port,
    input reg reg_port,
    output wire wire_port,

    input _abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLNMNOPQRSTUVWXYZ_1234567890_port
);
endmodule;
