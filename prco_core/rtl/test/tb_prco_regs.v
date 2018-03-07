`timescale 1ns / 1ps

`include "../inc/prco_test.v"

module tb_prco_regs;

    // Inputs
    reg i_clk;
    reg i_en;
    reg i_reset;
    reg [2:0] i_sela;
    reg [2:0] i_selb;
    reg i_we;
    reg [2:0] i_seld;
    reg [15:0] i_datd;

    // Outputs
    wire [15:0] q_data;
    wire [15:0] q_datb;

    always #10 i_clk = ~i_clk;

    // Instantiate the Unit Under Test (UUT)
    prco_regs uut (
        .i_clk(i_clk), 
        .i_en(i_en), 
        .i_reset(i_reset), 
        .i_sela(i_sela), 
        .q_data(q_data), 
        .i_selb(i_selb), 
        .q_datb(q_datb), 
        .i_we(i_we), 
        .i_seld(i_seld), 
        .i_datd(i_datd)
    );

    initial begin
        // Initialize Inputs
        i_clk = 0;
        i_en = 0;
        i_reset = 0;
        i_sela = 0;
        i_selb = 0;
        i_we = 0;
        i_seld = 0;
        i_datd = 0;
        
        @(posedge i_clk);
        i_en = 1;
        
        @(posedge i_clk);
        
        @(posedge i_clk);
        i_reset = 1;

        @(posedge i_clk);
        i_reset = 0;

        @(posedge i_clk);
        i_seld = 1;
        i_we = 1;
        i_datd = 16'hf0f0;
        
        @(posedge i_clk);
        i_we = 0;
        i_sela = 1;
        
        @(posedge i_clk);
        @(posedge i_clk);
        
        $finish;

    end
      
endmodule

