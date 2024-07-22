// tests/integration_test/uart/vsrc/UART_rx.sv
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details

module UART_rx #(parameter ADDR_WIDTH, parameter BAUD_PERIOD) (
input clk,
input rst_l,
input rx,
output [ADDR_WIDTH-1:0] rx_data,
output reg rx_done,
input clr_rx_done
);

reg rx_pre_stable, rx_stable, rx_post_stable;	// Meta stable FF outputs

logic start, dec_baud, shift, set_done;	// Intermediate signals
wire [11:0]baud_reset;
reg [11:0]baud_cnt;
logic [$clog2(ADDR_WIDTH)+1:0]bit_cnt;
logic [ADDR_WIDTH+1:0]rx_shft_reg;

typedef enum reg {IDLE, RECV} state_t;
state_t state, next_state;

// Meta stablility FF's for rx
always_ff @(posedge clk, negedge rst_l) begin
	if(~rst_l) begin
		rx_pre_stable <= 1'b1;
		rx_stable <= 1'b1;
		rx_post_stable <= 1'b1;
	end else begin
		rx_pre_stable <= rx;
		rx_stable <= rx_pre_stable;
		rx_post_stable <= rx_stable;
	end
	
end 

// BAUD counter
assign baud_reset = (start) ? BAUD_PERIOD >> 1 : BAUD_PERIOD;
always_ff @(posedge clk) begin
	if (start | shift) begin
		baud_cnt <= baud_reset;
		$display("verilog:rx: resetting baud time=%0t baud_reset=%0d baud_cnt=%0d start=%0d shift=%0d",$time,baud_reset, baud_cnt,start,shift);
	end
	else if (dec_baud) begin
		baud_cnt <= baud_cnt - 1;
		// $display("verilog:rx: decrement baud time=%0t baud_cnt=%0d",$time,baud_cnt);
	end
end

assign shift = (baud_cnt == '0) ? 1'b1 : 1'b0;

// Bit counter
always_ff@(posedge clk) begin
	if (start)
		bit_cnt <= '0;
	else if (shift) begin
		bit_cnt <= bit_cnt + 1;
		$display("verilog:rx: shfting value time=%0t rx_stable=%d bit_cnt=%d",$time, rx_stable, bit_cnt);
	end
		
end

// Shifter
always_ff @(posedge clk) begin
	if (shift)
		rx_shft_reg <= {rx_stable, rx_shft_reg[ADDR_WIDTH+1:1]};
end

assign rx_data = rx_shft_reg[ADDR_WIDTH:1];

// State machine logic
always_comb begin
	next_state = state;
	start = 1'b0;
	dec_baud = 1'b0;
	set_done = 1'b0;

	case (state)
	IDLE:
		if (rx_post_stable & ~rx_stable) begin	// Detect falling edge
			start = 1'b1;
			next_state = RECV;
			$display("verilog:rx: transition IDLE->RECV time=%0t rx_post_stable=%0d rx_stable=%0d", $time, rx_post_stable, rx_stable);
		end
	RECV:
		if (bit_cnt >= ADDR_WIDTH+2) begin
			set_done = 1'b1;
			next_state = IDLE;
			$display("verilog:rx: transition RECV->IDLE time=%0t rx_data=%0d", $time, rx_data);
		end else begin
			dec_baud = 1'b1;
		end
	default: begin
		$display("verilog:rx: bad state");
		$finish;
	end
	endcase
end

// State maching flop
always_ff @(posedge clk, negedge rst_l) begin
	if (~rst_l)
		state <= IDLE;
	else
		state <= next_state;
end

// Ready SR flop
always_ff @(posedge clk, negedge rst_l) begin
	if (~rst_l)
		rx_done <= 1'b0;
	else if (start | clr_rx_done) begin
		rx_done <= 1'b0;
	end
	else if (set_done)
		rx_done <= 1'b1;
end

endmodule;
