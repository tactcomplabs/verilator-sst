
// CHUNK_SIZE: size per chunk in bytes
`ifndef CHUNK_SIZE
`define CHUNK_SIZE 4
`endif
`ifndef NUM_CHUNKS
`define NUM_CHUNKS 2
`endif
`ifndef SCRATCHPAD_BASE
`define SCRATCHPAD_BASE 16
`endif
// NUM_CHUNKS: number of chunks
// SCRATCHPAD_BASE: base address
/* len:
    00 byte
    01 half
    10 word
    11 double
*/

module Scratchpad #(
    parameter CHUNK_SIZE=`CHUNK_SIZE,
    parameter NUM_CHUNKS=`NUM_CHUNKS,
    parameter SCRATCHPAD_BASE=`SCRATCHPAD_BASE)(
    input clk,
    input rst_l,
    input en,
    input write,
    input [63:0] addr,
    input [1:0] len,
    input [63:0] wdata,
    output [63:0] rdata
    );

localparam SCRATCHPAD_SIZE = CHUNK_SIZE * NUM_CHUNKS;
localparam ADDR_WIDTH = $clog2(SCRATCHPAD_SIZE);
reg [63:0] scratchpad_addr;

assign scratchpad_addr = addr - SCRATCHPAD_BASE;
assert property (@(posedge clk) scratchpad_addr < SCRATCHPAD_SIZE);
assert property (@(posedge clk) addr > SCRATCHPAD_BASE);

RAM_64 #(.ADDR_WIDTH(ADDR_WIDTH)) ram (
    .clk(clk),
    .en(en),
    .wr(write),
    .len(len),
    .addr(scratchpad_addr[ADDR_WIDTH-1:0]),
    .wdata(wdata),
    .rdata(rdata)
);

endmodule;
