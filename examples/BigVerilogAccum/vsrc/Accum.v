`ifndef ACCUM_WIDTH
`define ACCUM_WIDTH 48  //124
`endif


module Accum #(parameter ACCUM_WIDTH=`ACCUM_WIDTH) (
   input                       clk,
   input                       reset_l,
   input                       en,     // triggers addition
   input  [ACCUM_WIDTH-1:0]    add,    // value to be added
   output [ACCUM_WIDTH-1:0]    accum,  // current accumulated value
   output wire                 done    // signals addition completed
   );
   
   reg [ACCUM_WIDTH-1:0] accumulator;
   reg doner;

   always_ff @ (posedge clk, negedge reset_l) begin
      if (!reset_l) begin
         accumulator <= '0;
         doner <= '0;
      end
      else if (en && !doner) begin
         accumulator <= accumulator + add;
         doner <= 1'b1;
         //$display("verilog: accumulator: %x,%x\n", accumulator[ACCUM_WIDTH-1:64], accumulator[63:0]);
         //$display("verilog: add: %x,%x\n", add[ACCUM_WIDTH-1:64], add[63:0]);
      end
      //else if (en && doner) begin
         //$display("verilog: accumulator: %x,%x\n", accumulator[ACCUM_WIDTH-1:64], accumulator[63:0]);
         //$display("verilog: add: %x,%x\n", add[ACCUM_WIDTH-1:64], add[63:0]);
      //end
   end
   
   assign accum = accumulator;
   assign done = doner;

   initial begin
      $display("verilog: model running...\n");
   end

endmodule
