// test/pin Pin.sv
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

//Hardware IO Pin:
// send mode:
//     direction = 1
//     data_write = DATA,
// recv mode:
//     direction = 0
//     io_port = DATA

module Pin(
    input wire        direction,
    input wire [7:0]  data_write,
    output wire [7:0] data_read,
    inout wire [7:0]  io_port,
    input  clk
    );

reg [7:0] data_out;
reg [7:0] data_in;

assign io_port = direction ? data_out : 8'hzz;
assign data_read = data_in;

always @ (posedge clk)
begin
    data_in  <= io_port;
    data_out <= data_write;
end

endmodule
