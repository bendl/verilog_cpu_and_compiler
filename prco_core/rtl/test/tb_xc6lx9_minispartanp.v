`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   12:30:28 02/10/2018
// Design Name:   xc6lx9_msp
// Module Name:   Z:/uni/prco304/prco_core/rtl/test/tb_xc6lx9_minispartanp.v
// Project Name:  ise
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: xc6lx9_msp
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module tb_xc6lx9_minispartanp;

    // Inputs
    reg clk50;
    reg [7:0] LEDS;

    always #5 clk50 = ~clk50;

    // Instantiate the Unit Under Test (UUT)
    xc6lx9_msp uut (
        .clk50(clk50), 
        .LEDS(LEDS)
    );

    initial begin
        // Initialize Inputs
        clk50 = 0;
        LEDS = 0;

        // Wait 100 ns for global reset to finish
        #100;
        
        // Add stimulus here
        @(posedge clk50);
        @(posedge clk50);
        @(posedge clk50);
    end
      
endmodule

