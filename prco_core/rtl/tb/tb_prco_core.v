`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   19:02:24 02/09/2018
// Design Name:   prco_core
// Module Name:   /home/ise/XilinxVM/prco304/prco_core/rtl/tb//tb_prco_core.v
// Project Name:  ise
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: prco_core
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module tb_prco_core;

    // Inputs
    reg clk50;

    // Outputs
    wire [7:0] LEDS;

    // Instantiate the Unit Under Test (UUT)
    prco_core uut (
        .clk50(clk50), 
        .LEDS(LEDS)
    );

    initial begin
        // Initialize Inputs
        clk50 = 0;

        // Wait 100 ns for global reset to finish
        #100;
        
        // Add stimulus here

    end
      
endmodule

