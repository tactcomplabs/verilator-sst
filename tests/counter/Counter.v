`ifndef STOP_WIDTH
`define STOP_WIDTH 3
`endif


module Counter #(parameter STOP_WIDTH=`STOP_WIDTH) (
   input              clk,
   input              reset_l,
   input [STOP_WIDTH-1:0]       stop,
   output wire        done
   );
   
   reg [STOP_WIDTH-1:0] ctr;

   always_ff @ (posedge clk, negedge reset_l) begin
      if (!reset_l) begin
         ctr <= '0;
      end
      else begin
         ctr <= ctr + 1'b1;
         $display("verilog: counter: %0d\n", ctr);
      end
   end
   
   assign done = (ctr == stop);

   initial begin
      $display("verilog: model running...\n");
   end

endmodule
