// test/uart_mem UART_tx.sv
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
// See LICENSE in the top level directory for licensing details
//

module UART_tx #(parameter ADDR_WIDTH, parameter BAUD_PERIOD) (
    input clk, 
    input rst_l, 
    input trmt, 
    input [ADDR_WIDTH-1:0] tx_data, 
    output tx, 
    output tx_done,
	input clr_tx_done);

reg tx_done_reg;
logic [$clog2(ADDR_WIDTH)+1:0]bit_cnt;	// Counts each bit sent
logic [ADDR_WIDTH:0]shift_reg;	// Register to hold and shift tx data
logic init;	// SM output to begin transmission
logic transmitting;	// SM output to be high while transmitting
logic set_done;	// SM output to signal end of transmission
logic [11:0]baud_cnt;	// Count number of clock cycles till baud period is reached
logic shift;	// Signal to determine when the next bit should be shifted

typedef enum reg  {IDLE, TRANSMIT} state_t;	// SM states
state_t state, next_state;

// Bit counter
always_ff @(posedge clk) begin
	if (init)
		bit_cnt <= '0;
	else if (shift)
		bit_cnt <= bit_cnt + 1;
end

// Baud counter
always_ff @(posedge clk, negedge rst_l) begin
	if (init | shift | ~rst_l)begin
		baud_cnt <= '0;
		$display("verilog:tx: resetting baud time=%0t baud_cnt=%0d init=%0d shift=%0d",$time, baud_cnt,init,shift);
	end
	else if(transmitting) begin
		baud_cnt <= baud_cnt + 1;
		// $display("verilog:tx: increment baud time=%0t baud_cnt=%0d",$time,baud_cnt);
	end
end

// Determine shift logic
assign shift = (baud_cnt >= BAUD_PERIOD) ? 1'b1 : 1'b0;

// Shift register
always_ff @(posedge clk, negedge rst_l) begin
	if(~rst_l)
		shift_reg <= '1;	// asynch set
	else if (init)
		shift_reg <= {tx_data, 1'b0};
	else if (shift) begin
		shift_reg <= {1'b1, shift_reg[ADDR_WIDTH:1]};
		$display("verilog:tx: shfting value time=%0t tx=%d shift_reg=%d bit_cnt=%d",$time, tx, shift_reg, bit_cnt);
	end
end

// Assign output bit
assign tx = shift_reg[0];

// SM logic

always_comb begin
	next_state = state;
	init = 1'b0;
	transmitting = 1'b0;
	set_done = 1'b0;
	case (state)
	IDLE:
		if (trmt) begin
			init = 1'b1;
			next_state = TRANSMIT;
			$display("verilog:tx: transition IDLE->TRANSMIT time=%0t tx_data=%0d", $time, tx_data);
		end
	TRANSMIT:
		if (bit_cnt < ADDR_WIDTH+2) begin
			transmitting = 1'b1;
		end else begin
			set_done = 1'b1;
			next_state = IDLE;
			$display("verilog:tx: transition TRANSMIT->IDLE time=%0t tx_data=%0d", $time, tx_data);
		end
	endcase
end

// SM flop
always_ff @(posedge clk, negedge rst_l) begin
	if (~rst_l)
		state <= IDLE;
	else
		state <= next_state;
end

// Done SR flop
assign tx_done = tx_done_reg;
always_ff @(posedge clk, negedge rst_l) begin
	if (~rst_l)
		tx_done_reg <= 1'b0;
	else if (init | clr_tx_done)
		tx_done_reg <= 1'b0;
	else if (set_done)
		tx_done_reg <= 1'b1;
end

endmodule;
