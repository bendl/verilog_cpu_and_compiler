// prco_lmem.v

`include "inc/prco_constants.v"

module prco_lmem (
    input           i_clk,
    input           i_reset,
    
    // Pipeline control
    input               i_p_cp,
    input               i_p_stalled,
    input               i_p_valid,
    output              q_p_stalled,
    output reg          q_p_valid,
    input               i_p_ce,
    output              q_p_ce,

    input           i_mem_we,
    input [15:0]    i_mem_addr,
    input [15:0]    i_mem_dina,

    output [15:0]   q_mem_douta
);
    /// Define the on chip memory size
    parameter P_LMEM_DEPTH = 255;

    reg r_p_blocked = 0;
    
    reg [`REG_WIDTH:0] r_lmem[0:P_LMEM_DEPTH];
    integer i = 0;

    initial begin
        // Write initial memory contents for debug
        $display("r_lmem:");
        for (i = 0; i < 10; i = i + 1) begin
            r_lmem[i] = 0;
            $display("RAM[0x%h] = %h", i, r_lmem[i]);
        end
        // Print out top value
        $display("RAM[0x%h] = %h", 
            P_LMEM_DEPTH, r_lmem[P_LMEM_DEPTH]);
        
        // Debug instructions
        r_lmem[0] = 16'h20ab; // MOVI 0, ab
        r_lmem[1] = 16'h21cd; // MOVI 1, cd
        r_lmem[2] = 16'h0000; // NOP
        r_lmem[3] = 16'h22ef; // MOVI 2, ef
    end

    always @(posedge i_clk) begin
        if (i_mem_we == 1) begin
            $display("Writing 0x%h to RAM[0x%h]", 
                i_mem_dina, i_mem_addr);
            r_lmem[i_mem_addr] <= i_mem_dina;
        end
    end

    assign q_mem_douta = r_lmem[i_mem_addr];

    // Pipeline control
    // We are stalled if we are ready but the next stage isn't ready (stalled)
    assign q_p_stalled  = q_p_valid && (i_p_stalled || (r_p_blocked));

    //assign    stage[n]_stalled = (stage[n]_valid) && 
    // (
        // (stage[n+1]_stalled)
    //  ||(things that would stall this stage)
    //);

    // Ready to progress if: previous stage is ready (valid)
    // and next stage isn't stalled.
    assign q_p_ce       = i_p_valid && !q_p_stalled;

    always @(posedge i_clk) begin
        if (q_p_stalled) begin
            $display("MEM: Stalled because of core!");
        end

        if (q_p_ce) begin
            $display("MEM: doing...");
        end
    end

    always @(posedge i_clk) begin
        if (i_reset || i_p_cp) begin
            q_p_valid <= 0;
        end else if (q_p_ce) begin
            q_p_valid <= i_p_valid;
        end else if (i_p_ce) begin
            q_p_valid <= 0;
        end
    end

endmodule