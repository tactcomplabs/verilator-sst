`ifndef ACCUM_WIDTH
`define ACCUM_WIDTH 32
`define ADD_WIDTH ACCUM_WIDTH/2
`endif


module Accum #(parameter ACCUM_WIDTH=`ACCUM_WIDTH, ADD_WIDTH=`ADD_WIDTH) (
   input                       clk,
   input                       reset_l,
   input                       en,     // triggers addition
   input  [ADD_WIDTH-1:0]   add[4],  // value to be added
   output [ACCUM_WIDTH-1:0] accum[4],  // current accumulated value
   output wire                 done    // signals addition completed
   );

   reg [ACCUM_WIDTH-1:0] accumulator[4];
   reg doner;
   integer i;

   always_ff @ (posedge clk, negedge reset_l) begin
      if (!reset_l) begin
         for (i=0; i<4; i++) 
         begin
           accumulator[i] <= '0;
         end
         doner <= '0;
      end
      else if (en && !doner) begin
         for (i=0; i<4; i++)
         begin
            accumulator[i] <= accumulator[i] + {{ADD_WIDTH{1'b0}}, add[i]};
            $display("verilog: accumulator[%d]: %x\n", i, accumulator[i]);
            $display("verilog: add[%d]: %x\n", i, add[i]);
         end
         doner <= 1'b1;
      end
      else if (!en && doner) begin
         doner <= 1'b0;
      end
   end

   assign accum = accumulator;
   assign done = doner;

   initial begin
      $display("verilog: model running...\n");
   end

endmodule
