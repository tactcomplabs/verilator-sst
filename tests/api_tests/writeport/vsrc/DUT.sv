// tests/api_tests/writeport/vsrc/DUT.sv
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details

module DUT(
    input          write_port,
    input [7:0]    write_byte_port,
    input [15:0]   write_half_port,
    input [31:0]   write_word_port,
    input [63:0]   write_double_port,
    input [127:0]  write_quad_port,
    input          write_array_port [0:127],
    input [127:0]  write_quad_array_port [0:127],
    input reg      write_reg_port,
    input wire     write_wire_port,
    output         read_write_port,
    output [7:0]   read_write_byte_port,
    output [15:0]  read_write_half_port,
    output [31:0]  read_write_word_port,
    output [63:0]  read_write_double_port,
    output [127:0] read_write_quad_port,
    output         read_write_array_port [0:127],
    output [127:0] read_write_quad_array_port [0:127],
    output         read_write_reg_port,
    output         read_write_wire_port
);

assign read_write_port = write_port;
assign read_write_byte_port = write_byte_port;
assign read_write_half_port = write_half_port;
assign read_write_word_port = write_word_port;
assign read_write_double_port = write_double_port;
assign read_write_quad_port = write_quad_port;
genvar i;
generate
    for( i = 0; i < $size(write_array_port); i++ ) begin
        assign read_write_array_port[i] = write_array_port[i];
        assign read_write_quad_array_port[i] = write_quad_array_port[i];
    end
endgenerate
assign read_write_reg_port = write_reg_port;
assign read_write_wire_port = write_wire_port;
endmodule;
