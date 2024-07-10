module Array_IO (
    input [23:0] arr_in [0:9], // 3B x 10
    output [23:0] arr_out [0:9]
);
genvar i;
generate
    for(i=0;i<10;i=i+1) begin
        assign arr_out[i] = ~arr_in[i];
    end
endgenerate
endmodule;
