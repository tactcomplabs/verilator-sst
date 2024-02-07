`ifndef START
`define START 10
`endif
`ifndef INPUT_WIDTH
`define INPUT_WIDTH 32
`endif

/* verilator lint_off DECLFILENAME */
module Top
  (
   // Declare some signals so we can see how I/O works
   input              clk,
   input              reset_l,
   input [INPUT_WIDTH-1:0]       stop,
   output wire        done
   );
   
   reg [INPUT_WIDTH-1:0] ctr;
   reg [INPUT_WIDTH-1:0] safe_stop;
   wire [INPUT_WIDTH-1:0] next_ctr;

   always_ff @ (posedge clk) begin
      if (!reset_l) begin
         ctr <= `START;
         safe_stop <= stop;
      end
      else begin
         ctr <= next_ctr;
         $display("verilog: counter: %0d\n", ctr);
      end
   end
   
   assign done = (ctr == safe_stop);
   assign next_ctr = ctr + 1'b1;

   initial begin
      $display("verilog: model running...\n");
   end

endmodule
