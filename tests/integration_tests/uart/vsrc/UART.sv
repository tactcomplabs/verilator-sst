// tests/integration_test/uart/vsrc/UART.sv
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details

//addr_width > data_width
//nRX=nTX
//read rx={0} -> rx{addr} -> tx{data}
//write rx={1} -> rx{addr} -> rx{data}

`ifndef ADDR_WIDTH
`define ADDR_WIDTH 1
`endif

`ifndef DATA_WIDTH
`define DATA_WIDTH 1
`endif

`ifndef BAUD_PERIOD
`define BAUD_PERIOD 1
`endif

`define assert(cond,msg) initial begin \
    if (~cond) begin \
        $display(msg); \
        $finish; \
    end \
end

module UART #(
    parameter ADDR_WIDTH=`ADDR_WIDTH, 
    parameter DATA_WIDTH=`DATA_WIDTH, 
    parameter BAUD_PERIOD=`BAUD_PERIOD)(
    input clk,
    input rst_l,
    input RX,
    output TX,
    output [DATA_WIDTH-1:0] mem_debug [0:(2**ADDR_WIDTH)-1]);

wire rx_done, tx_done;
wire [ADDR_WIDTH-1:0] tx_data, rx_data;
reg [ADDR_WIDTH-1:0] write_addr;
reg set_addr, wr, trmt, clr_rx_done, clr_tx_done;

UART_tx #(.ADDR_WIDTH(ADDR_WIDTH), .BAUD_PERIOD(BAUD_PERIOD)) iTX(
    .clk(clk),
    .rst_l(rst_l),
    .tx(TX),
    .trmt(trmt),
    .tx_data(tx_data),
    .tx_done(tx_done),
    .clr_tx_done(clr_tx_done));

UART_rx #(.ADDR_WIDTH(ADDR_WIDTH), .BAUD_PERIOD(BAUD_PERIOD)) iRX(
    .clk(clk), 
    .rst_l(rst_l), 
    .rx(RX), 
    .rx_data(rx_data),
    .rx_done(rx_done),
    .clr_rx_done(clr_rx_done));


wire [DATA_WIDTH-1:0] rdata;
assign tx_data = {{ADDR_WIDTH-DATA_WIDTH{1'bx}}, rdata};
RAM #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH)) ram (
    .clk(clk),
    .rst_l(rst_l),
    .wr(wr),
    .addr(write_addr),
    .wdata(rx_data[DATA_WIDTH-1:0]),
    .rdata(rdata),
    .mem_debug(mem_debug));

typedef enum reg [2:0] {IDLE, GET_ADDR, GET_DATA, PUT_ADDR, PUT_DATA} wrapper_state_t;
wrapper_state_t state, next_state;

always_ff @(posedge clk, negedge rst_l) begin
	if (~rst_l)
		write_addr <= '0;
	else if (set_addr)
        write_addr <= rx_data;
end

always_ff @(posedge clk, negedge rst_l) begin
	if (~rst_l)
		state <= IDLE;
	else
        state <= next_state;
end

always_comb begin
    next_state = state;
    set_addr = 1'b0;
    wr = 1'b0;
    clr_rx_done = 1'b0;
    clr_tx_done = 1'b0;
    trmt = 1'b0;

    case(state)
    IDLE:
        if (rx_done) begin
            $display("verilog:top: time=%0t rx_done=%0d",$time,rx_done);
            if ((rx_data & 1) === 1) begin
                clr_rx_done = 1'b1;
                next_state = PUT_ADDR;
                $display("verilog:top: transition IDLE->PUT_ADDR time=%0t",$time);
            end
            else begin
                clr_rx_done = 1'b1;
                next_state = GET_ADDR;
                $display("verilog:top: transition IDLE->GET_ADDR time=%0t",$time);
            end
        end
    PUT_ADDR:
        if (rx_done) begin
            clr_rx_done = 1'b1;
            set_addr = 1'b1;
            next_state = PUT_DATA;
            $display("verilog:top: transition PUT_ADDR->PUT_DATA time=%0t",$time);
        end
    PUT_DATA:
        if (rx_done) begin
            clr_rx_done = 1'b1;
            wr = 1'b1;
            next_state = IDLE;
            $display("verilog:top: transition PUT_DATA->IDLE time=%0t",$time);
        end
    GET_ADDR:
        if (rx_done) begin
            clr_rx_done = 1'b1;
            set_addr = 1'b1;
            next_state = GET_DATA;
            $display("verilog:top: transition GET_ADDR->GET_DATA time=%0t rx_data=%0d tx_done=%0d",$time,rx_data,tx_done);
        end
    GET_DATA:
        if (tx_done) begin
            clr_tx_done = 1'b1;
            next_state = IDLE;
            $display("verilog:top: transition GET_DATA->IDLE time=%0t",$time);
        end else begin
            trmt = 1'b1;
        end
    default: begin
		$display("verilog:top: bad state time=%0t",$time);
		$finish;
	end
    endcase
end

initial begin
    if (ADDR_WIDTH < DATA_WIDTH) begin
        $display("ADDR_WIDTH must be larger than or equal to DATA_WIDTH");
        $finish;
    end
    $display("ADDR_WIDTH=%0d DATA_WIDTH=%0d BAUD_PERIOD=%0d", ADDR_WIDTH, DATA_WIDTH, BAUD_PERIOD);
end

endmodule;
