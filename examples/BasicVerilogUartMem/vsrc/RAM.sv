module RAM #(parameter ADDR_WIDTH, parameter DATA_WIDTH)(
    input clk,
    input rst_l,
    input wr,
    input [ADDR_WIDTH-1:0] addr,
    input [DATA_WIDTH-1:0] wdata,
    output [DATA_WIDTH-1:0] rdata,
    output [DATA_WIDTH-1:0] mem_debug [0:(2**ADDR_WIDTH)-1] 
);

reg [DATA_WIDTH-1:0] mem [0:(2**ADDR_WIDTH)-1];
assign mem_debug = mem;

always_ff @ (posedge clk, negedge rst_l) begin
      if (~rst_l) begin
        mem <= '{default:0};
      end
      else if (wr) begin
        mem[addr] <= wdata;
        $display("verilog:ram: wrote %0d to mem addr %0d",wdata,addr);
      end
   end

assign rdata = ~wr ? mem[addr] : 'z;

endmodule;
