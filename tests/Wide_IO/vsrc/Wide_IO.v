module Wide_IO (
    input [63:0] long_in,
    input [127:0] long_long_in,
    input [511:0] super_in,
    output [63:0] long_out,
    output [127:0] long_long_out,
    output [511:0] super_out
);

assign long_out = ~long_in;
assign long_long_out = ~long_long_in;
assign super_out = ~super_in;

endmodule;