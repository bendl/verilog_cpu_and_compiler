`timescale 1ns / 1ps

`include "../inc/prco_constants.v"
`include "../inc/prco_isa.v"
`include "../inc/prco_test.v"

module tb_prco_decoder;

    // Inputs
    reg i_clk;
    reg i_en;
    reg [15:0] i_instr;

    // Outputs
    wire [4:0] q_op;
    wire [2:0] q_seld;
    wire [2:0] q_sela;
    wire       q_reg_we;

    // Create clock signal
    always #10 i_clk = ~i_clk;

    // Instantiate the Unit Under Test (UUT)
    prco_decoder uut (
        .i_clk(i_clk), 
        .i_en(i_en), 
        .i_instr(i_instr), 
        .q_op(q_op), 
        .q_seld(q_seld), 
        .q_sela(q_sela),
        .q_reg_we(q_reg_we)
    );

    initial begin
        // Initialize Inputs
        i_clk = 0;
        i_en = 0;
        i_instr[`PRCO_OP_BITS] = `PRCO_OP_NOP;
        i_instr[`PRCO_SELD_BITS] = 0;
        i_instr[`PRCO_IMM8_BITS] = 8'h00;
        
        @(posedge i_clk);
        i_en = 1;
        @(posedge i_clk);

        // MOVI instr
        i_instr[15:11] = `PRCO_OP_MOVI;
        i_instr[10:8] = 0;
        i_instr[7:0] = 8'hAB;
        @(posedge i_clk);
        `assert(q_op == `PRCO_OP_MOVI);
        `assert(q_reg_we == 1);
        
        // NOP
        i_instr[15:11] = `PRCO_OP_NOP;
        i_instr[10:8] = 0;
        i_instr[7:0] = 8'h00;
        @(posedge i_clk);
        `assert(q_op == `PRCO_OP_NOP);
        `assert(q_reg_we == 0);

        // MOVI instr
        i_instr[15:11] = `PRCO_OP_MOVI;
        i_instr[10:8] = 1;
        i_instr[7:0] = 8'hCD;
        @(posedge i_clk);
        `assert(q_op == `PRCO_OP_MOVI);
        `assert(q_reg_we == 1);

        // NOP
        i_instr[15:11] = `PRCO_OP_NOP;
        i_instr[10:8] = 0;
        i_instr[7:0] = 8'h00;
        @(posedge i_clk);
        `assert(q_op == `PRCO_OP_NOP);
        `assert(q_reg_we == 0);

        // MOV
        i_instr[15:11] = `PRCO_OP_MOV;
        i_instr[10:8] = 1;
        i_instr[7:5] = 2;
        i_instr[4:0] = 5'h0;
        @(posedge i_clk);
        `assert(q_op == `PRCO_OP_MOV);
        `assert(q_seld == 1);
        `assert(q_sela == 2);
        `assert(q_reg_we == 1);

        $finish;

    end
      
endmodule

