`timescale 1ns / 1ps

module tb_xc6lx9_minispartanp;

    // Inputs
    reg clk50;
    reg [7:0] LEDS;
	reg PORTC3;

    always #10 clk50 = ~clk50;

    // Instantiate the Unit Under Test (UUT)
    xc6lx9_msp uut (
        .clk50(clk50),
		.PORTC3(PORTC3),		
        .LEDS(LEDS)
    );

    initial begin
        // Initialize Inputs
        clk50 = 0;
        LEDS = 0;
		PORTC3 = 0;
        
        // Add stimulus here
        @(posedge clk50);
        @(posedge clk50);
        @(posedge clk50);
		
		PORTC3 = 1;
        @(posedge clk50);
		PORTC3 = 0;
        @(posedge clk50);
		PORTC3 = 1;
        @(posedge clk50);
		
		#100;
		
		PORTC3 = 0;
        @(posedge clk50);
		PORTC3 = 1;
        @(posedge clk50);
		PORTC3 = 0;
        @(posedge clk50);
        @(posedge clk50);
        @(posedge clk50);
        @(posedge clk50);
        @(posedge clk50);
        @(posedge clk50);
		
		
		#1000000
		PORTC3 = 1;
        @(posedge clk50);
        @(posedge clk50);
        @(posedge clk50);
		PORTC3 = 0;
        @(posedge clk50);
        @(posedge clk50);
        @(posedge clk50);
		PORTC3 = 1;
		
		
		
		#1000000
		PORTC3 = 0;
		
    end
      
endmodule

