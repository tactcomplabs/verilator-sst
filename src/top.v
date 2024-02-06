// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed under the Creative Commons Public Domain, for
// any use, without warranty, 2003 by Wilson Snyder.
// SPDX-License-Identifier: CC0-1.0
// ======================================================================

// This is intended to be a complex example of several features, please also
// see the simpler examples/make_hello_c.

/* verilator lint_off DECLFILENAME */
module Top
   #(parameter MAX_WIDTH=32)
  (
   // Declare some signals so we can see how I/O works
   input              clk,
   input              reset_l,
   input [MAX_WIDTH-1:0]       max,
   output wire        done
   );
   
   reg [MAX_WIDTH-1:0] ctr;
   reg [MAX_WIDTH-1:0] safe_max;
   wire [MAX_WIDTH-1:0] next_ctr;

   always_ff @ (posedge clk) begin
      if (!reset_l) begin
         ctr <= '0;
         safe_max <= `MAX;
      end
      else begin
         ctr <= next_ctr;
         $display("verilog: counter: %0d %0d %0d\n", ctr, max, `MAX);
      end
   end
   
   assign done = (ctr == safe_max);
   assign next_ctr = ctr + 1'b1;

   initial begin
      $display("verilog: model running...\n", $time);
   end

endmodule
