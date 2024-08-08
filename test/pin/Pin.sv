module Pin(
    input  clk,
    input  direction,
    input  [7:0] data_write,
    output [7:0] data_read,
    inout  [7:0] io_port
    );

reg [7:0] data_out;
reg [7:0] data_in;

assign io_port   = direction ? data_out : 'z;
assign data_read = data_in;

always @ (posedge clk)
begin
    data_in  <= io_port;
    data_out <= data_write;
end

endmodule
