`timescale 1ns / 1ps

`include "../inc/prco_constants.v"
`include "../inc/prco_test.v"

module tb_prco_lmem;

    // Inputs
    reg i_clk;
    reg i_mem_we;
    reg [15:0] i_mem_addr;
    reg [15:0] i_mem_dina;

    // Outputs
    wire [15:0] q_mem_douta;

    always #10 i_clk = ~i_clk;

    // Instantiate the Unit Under Test (UUT)
    prco_lmem #(32) 
    uut (
        .i_clk(i_clk), 
        .i_mem_we(i_mem_we), 
        .i_mem_addr(i_mem_addr), 
        .i_mem_dina(i_mem_dina), 
        .q_mem_douta(q_mem_douta)
    );

    initial begin
        // Initialize Inputs
        i_clk = 0;
        i_mem_we = 0;
        i_mem_addr = 0;
        i_mem_dina = 0;

        // Add stimulus here
        @(posedge i_clk);

        i_mem_addr = 0;
        i_mem_dina = 16'hab;
        i_mem_we = 1;
        @(posedge i_clk);
        `assert(q_mem_douta == 16'hab);

        i_mem_addr = 1;
        i_mem_dina = 16'hcd;
        i_mem_we = 1;
        @(posedge i_clk);
        `assert(q_mem_douta == 16'hcd);

        i_mem_we = 0;
        i_mem_addr = 0;
        i_mem_dina = 16'h0;
        @(posedge i_clk);
        `assert(q_mem_douta == 16'hcd);

        $finish;
    end
      
endmodule

