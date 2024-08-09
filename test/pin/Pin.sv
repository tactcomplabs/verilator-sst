//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
// Pin.sv

//Hardware IO Pin:
// send mode:
//     direction = 1
//     data_write = DATA,
// recv mode:
//     direction = 0
//     io_port = DATA

module Pin(
    input  direction,
    input  [7:0] data_write,
    output [7:0] data_read,
    inout wire [7:0] io_port,
    input  clk
    );

reg [7:0] data_out;
reg [7:0] data_in;
wire [7:0] moubarak;
assign moubarak = data_out;
assign io_port = data_out;
assign data_read = data_in;

always @ (posedge clk)
begin
    data_in  = io_port;
    data_out = data_write;
    $display("\nverilog: tick=%t moubarak=0x%h\n",$time,    moubarak);
    $display("\nverilog: tick=%t data_in=0x%h\n",$time,    data_in);
    $display("\nverilog: tick=%t data_out=0x%h\n",$time,   data_out);
    $display("\nverilog: tick=%t data_write=0x%h\n",$time, data_write);
    $display("\nverilog: tick=%t data_read=0x%h\n",$time,  data_read);
    $display("\nverilog: tick=%t direction=0x%h\n",$time,  direction);
    $display("\nverilog: tick=%t io_port=0x%h\n",$time,    io_port);
end

always @ (negedge clk)
begin
    $display("\nverilog: tick=%t moubarak=0x%h\n",$time,    moubarak);
    $display("\nverilog: tick=%t data_in=0x%h\n",$time,    data_in);
    $display("\nverilog: tick=%t data_out=0x%h\n",$time,   data_out);
    $display("\nverilog: tick=%t data_write=0x%h\n",$time, data_write);
    $display("\nverilog: tick=%t data_read=0x%h\n",$time,  data_read);
    $display("\nverilog: tick=%t direction=0x%h\n",$time,  direction);
    $display("\nverilog: tick=%t io_port=0x%h\n",$time,    io_port);
end
initial begin
    $display("\nverilog: tick=%t moubarak=0x%h\n",$time,    moubarak);
    $display("\nverilog: tick=%t data_in=0x%h\n",$time,    data_in);
    $display("\nverilog: tick=%t data_out=0x%h\n",$time,   data_out);
    $display("\nverilog: tick=%t data_write=0x%h\n",$time, data_write);
    $display("\nverilog: tick=%t data_read=0x%h\n",$time,  data_read);
    $display("\nverilog: tick=%t direction=0x%h\n",$time,  direction);
    $display("\nverilog: tick=%t io_port=0x%h\n",$time,    io_port);
end

endmodule
