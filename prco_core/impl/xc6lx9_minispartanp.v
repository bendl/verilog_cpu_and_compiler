// prco_core

`define PORTC3_LGWAIT 16

module xc6lx9_msp(
    input clk50,
    input [3:0] DIPSW,
    
	// Button, Active HIGH
	input PORTC3,
	 
    // uart tx 
    output PORTC10,
    output PORTC11,
    
    output [7:0] LEDS
);

	reg     inst_core_enable = 1;
    wire    q;

    wire    w_leds;
    
    reg r_top_reset = 1;
    wire r_top_debug_instr_clk;
    
    wire [7:0] r_top_q_debug;
    
    // uart logic
    wire [7:0] r_uart_tx_byte;

    // Instantiate the module
    prco_core inst_core (
        .i_clk(clk50), 
        .i_en(inst_core_enable), 
        .i_reset(r_top_reset),
    
        .i_rx(r_uart_i_rx),
        .q_tx(r_uart_q_tx),
        .q_tx_byte(r_uart_tx_byte),
        .q_debug_instr_clk(r_top_debug_instr_clk),
        .q_debug(r_top_q_debug)
    );
    
    always @(posedge clk50) begin
        if(r_top_reset) begin
            r_top_reset <= 0;
        end
    end
    
    assign LEDS = { 
		//{4{1'b0}}, 
		test_bin,
		test_state, 
		r_uart_q_tx, 
		clk50, 
		r_top_debug_instr_clk
	};
	
    assign PORTC11 = r_uart_q_tx;
    assign PORTC10 = r_uart_i_rx;
	 
	// Debounce PORTC3 (instruction stepper)
	reg test_state = 1'b0;
	reg [3:0] test_bin = 4'b0001;
	reg sync_pipe;
	initial	sync_pipe      = 1'b0;
	reg r_button_state;
	initial	r_button_state = 1'b0;
	reg [`PORTC3_LGWAIT-1:0] timer;
	
	reg r_last;
	reg r_button_event;
	initial	r_last         = 1'b0;
	initial	r_button_event = 1'b0;
	
	always @(posedge clk50)
		{ r_button_state, sync_pipe }
			<= { sync_pipe, PORTC3 };
	
	initial timer = {(`PORTC3_LGWAIT){1'b1}};
	reg o_debounced = 0;
	always @(posedge clk50) begin
		timer <= timer - 1'b1;
		if(timer == 0) begin
			o_debounced <= r_button_state;
		end
	end
	
	always @(posedge clk50)
	begin
		r_last <= o_debounced;
		r_button_event <= (o_debounced)&&(!r_last);
	end
	
	always @(posedge clk50) begin
		if(r_button_event)
			test_state <= !test_state;
	end
	
	always @(posedge clk50)
	begin
		if(r_button_event) begin
			test_bin <= test_bin + 1;
		end
	end

endmodule