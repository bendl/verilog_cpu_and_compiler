// prco_lmem.v

`include "inc/prco_constants.v"

module prco_lmem (
    input           i_clk,

    // Pipeline signals
    input           i_ce,

    input           i_mem_we,
    input [15:0]    i_mem_addr,
    input [15:0]    i_mem_dina,

    output reg          q_mem_done,
    output reg [15:0]   q_mem_douta
);
    /// Define the on chip memory size
    parameter P_LMEM_DEPTH = 255;
    
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
        r_lmem[3] = 16'h0800; // LW 0, 00
        r_lmem[4] = 16'h0000; // NOP
        r_lmem[5] = 16'h22ef; // MOVI 2, ef

        r_lmem[P_LMEM_DEPTH] = 16'h00ff; // MOVI 2, ef
    end

    always @(posedge i_clk) begin
        if(i_ce) begin
            if (i_mem_we == 1) begin
                $display("Writing 0x%h to RAM[0x%h]", 
                    i_mem_dina, i_mem_addr);
                r_lmem[i_mem_addr] <= i_mem_dina;
            end

            q_mem_douta <= r_lmem[i_mem_addr];
        end
    end
    //assign q_mem_douta = r_lmem[i_mem_addr];

endmodule