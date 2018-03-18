`timescale 1ns / 1ps

module tb_prco_core;

    // Inputs
    reg i_clk;
    reg i_en;
    reg i_reset;
    
    reg i_mode;
    reg i_step;
    
    reg q_p_ce = 1;

    reg dec_block = 0;

    // Outputs
    wire [7:0] q_debug;
    wire       q_debug_instr_clk;

    // Create clock signal
    always #10 i_clk = ~i_clk;

    // Instantiate the Unit Under Test (UUT)
    prco_core uut (
        .i_clk(i_clk), 
        .i_en(i_en), 
        .i_reset(i_reset), 
        
        .i_mode(i_mode),
        .i_step(i_step),

        .q_debug_instr_clk(q_debug_instr_clk),
        .q_debug(q_debug)
    );

    initial begin
        // Initialize Inputs
        i_clk = 0;
        i_en = 0;
        i_reset = 1;
        i_mode = 0;
        i_step = 0;
		
		@(posedge i_clk);
		@(posedge i_clk);
		@(posedge i_clk);
		i_reset = 0;
		@(posedge i_clk);
		@(posedge i_clk);
		@(posedge i_clk);
    end
endmodule

