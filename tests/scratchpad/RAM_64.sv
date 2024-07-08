module RAM_64 #(parameter ADDR_WIDTH)(
    input clk,
    input en,
    input wr,
    input [1:0] len,
    input [ADDR_WIDTH-1:0] addr,
    input [63:0] wdata,
    output [63:0] rdata
);

reg [7:0] mem [0:(2**ADDR_WIDTH)-1];

wire [63:0] mask;
assign mask[63:32] = 32'hFFFFFFFF & {32{&len}};
assign mask[31:16] = 16'hFFFF & {16{len[1]}};
assign mask[15:8] = 8'hFF & {8{|len}};
assign mask[7:0] = 8'hFF;

// write operation
genvar i;
generate
  for (i=0;i<8;i=i+1) begin
    always_ff @(posedge clk) begin
      if (wr & en) begin
        mem[addr+i] <= (wdata[(8*i)+7:(8*i)] & mask[(8*i)+7:(8*i)]) | (mem[addr+i] & ~mask[(8*i)+7:(8*i)]);
      end
    end
  end
endgenerate

// read operation
genvar j;
generate
  for (j=0;j<8;j=j+1) begin
    always_ff @(posedge clk) begin
      if (~wr & en) begin
        rdata[(8*j)+7:(8*j)] <= mem[addr+j] & mask[(8*j)+7:(8*j)];
      end
    end
  end
endgenerate

always_ff@(negedge clk) begin
  if(wr & en)
    $display("verilog:wdata=%h addr=%d",wdata, addr);
  else if(~wr & en)
    $display("verilog:rdata=%h addr=%d",rdata, addr);
end

endmodule;
