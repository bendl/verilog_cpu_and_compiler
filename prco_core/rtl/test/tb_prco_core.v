`timescale 1ns / 1ps

module tb_prco_core;

	// Inputs
	reg i_clk;
	reg i_en;
	reg i_reset;
	reg i_p_stalled;
	reg q_p_ce = 1;

	reg dec_block = 0;

	// Outputs
	wire [7:0] q_debug;

    // Create clock signal
    always #5 i_clk = ~i_clk;

	// Instantiate the Unit Under Test (UUT)
	prco_core uut (
		.i_clk(i_clk), 
		.i_en(i_en), 
		.i_reset(i_reset), 

		.i_p_stalled(i_p_stalled),
		.i_p_dec_block(dec_block),

		.q_debug(q_debug)
	);

	initial begin
		// Initialize Inputs
		i_clk = 0;
		i_en = 0;
		i_reset = 1;
		i_p_stalled = 1;
        
		// Add stimulus here
        @(posedge i_clk);
        i_en = 1;
        i_reset = 1;
		i_p_stalled = 1;
        
        @(posedge i_clk);
        i_reset = 0;
		i_p_stalled = 1;
        
        @(posedge i_clk);
        
        @(posedge i_clk);
		i_p_stalled = 0;
        
        @(posedge i_clk);
        
        @(posedge i_clk);

        @(posedge i_clk);
        
        @(posedge i_clk);
        //dec_block = 1;
        
        @(posedge i_clk);
        
        @(posedge i_clk);
        
        @(posedge i_clk);

        @(posedge i_clk);
        @(posedge i_clk);
        @(posedge i_clk);
        @(posedge i_clk);
        @(posedge i_clk);
        //dec_block = 0;
        
        @(posedge i_clk);
        @(posedge i_clk);
        @(posedge i_clk);
        @(posedge i_clk);
        @(posedge i_clk);

        @(posedge i_clk);
        @(posedge i_clk);
        @(posedge i_clk);
        @(posedge i_clk);
        @(posedge i_clk);
        
		$finish;
	end
      
endmodule

