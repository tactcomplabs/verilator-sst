// test/accum1D Accum1D.sv
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

`ifndef ACCUM_WIDTH
`define ACCUM_WIDTH 256
`define ADD_WIDTH ACCUM_WIDTH/2
`endif


module Accum1D #(parameter ACCUM_WIDTH=`ACCUM_WIDTH, ADD_WIDTH=`ADD_WIDTH) (
   input                       clk,
   input                       reset_l,
   input                       en,     // triggers addition
   input  [ADD_WIDTH-1:0]   add,  // value to be added
   output [ACCUM_WIDTH-1:0] accum,  // current accumulated value
   output wire                 done    // signals addition completed
   );

   reg [ACCUM_WIDTH-1:0] accumulator;
   reg doner;
   integer i;

   always_ff @ (posedge clk, negedge reset_l) begin
      if (!reset_l) begin
         accumulator <= '0;
         doner <= '0;
      end
      else if (en && !doner) begin
         doner <= 1'b1;
         accumulator <= accumulator + {{ADD_WIDTH{1'b0}}, add};
         $display("verilog: accumulator: %x\n", accumulator);
         $display("verilog: add: %x\n", add);
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
