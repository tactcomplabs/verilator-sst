module DUT(
    output read_port,
    output [127:0] read_quad_port,
    output read_array_port [0:127],
    output [127:0] read_quad_array_port [0:127],
    output reg read_reg_port,
    output wire read_wire_port
);

assign read_port = 1'b1;
assign read_wire_port = 1'b1;
assign read_quad_port = 128'h0123_4567_89AB_CDEF_FEDC_BA98_7654_3210;

genvar i;
generate
    for(i = 0; i < 128; i = i + 1) begin

        assign read_quad_array_port[i] = 128'h0123_4567_89AB_CD00_FEDC_BA98_7654_3200 | {{120{1'b0}},i[7:0]} | {{120{1'b0}},i[7:0]} << 64;
        assign read_array_port[i] = i[0];
    end
endgenerate

initial begin
    read_reg_port = 1'b1;
end

endmodule;
