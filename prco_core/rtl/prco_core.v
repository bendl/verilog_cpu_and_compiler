// prco_core

module prco_core(
    input i_clk,
    input i_en,
    input i_reset,

    output [7:0] q_debug
);
    // program counter
    reg [15:0] pc = 0;

    reg [5:0] states = 6'h1;
    
    reg         r_dec_en = 1;
    wire [5:0]  r_dec_op;
    wire [2:0]  r_dec_seld;
    wire [2:0]  r_dec_sela;
    wire        r_dec_we;
    wire [15:0] r_dec_imm8;

    wire [15:0] r_reg_doutd;
    wire [15:0] r_reg_douta;
    reg         r_reg_en = 1;

    reg [15:0]  r_mem_addr;
    always @(posedge i_clk) begin
        if (states[0] == 1) begin
            r_mem_addr = pc;
        end else begin
            r_mem_addr = 0;
        end
    end
    
    wire [15:0] r_mem_douta;

    always @(posedge i_clk, posedge i_reset) begin
        if (i_reset == 1) begin
            pc <= 0;
            states = 6'h1;
        end else begin
            pc <= pc + 1;
        end
    end

    // Instantiate the module
    prco_decoder inst_decoder (
        .i_clk(i_clk), 
        .i_en(r_dec_en), 
        .i_instr(r_mem_douta), 
        .q_op(r_dec_seld_op), 
        .q_seld(r_dec_seld), 
        .q_sela(r_dec_sela),
        .q_imm8(r_dec_imm8),
        .q_reg_we(r_dec_we)
        );

    // Instantiate the module
    prco_regs inst_regs (
        .i_clk(i_clk), 
        .i_en(r_reg_en), 
        .i_reset(i_reset), 
        .i_sela(r_dec_seld), 
        .q_data(r_reg_doutd), 
        .i_selb(r_dec_sela), 
        .q_datb(r_reg_douta), 
        .i_we(r_dec_we), 
        .i_seld(r_dec_seld), 
        .i_datd(r_dec_imm8)
        );

    // Instantiate the module
    prco_lmem inst_lmem (
        .i_clk(i_clk), 
        .i_mem_we(i_mem_we), 
        .i_mem_addr(r_mem_addr), 
        .i_mem_dina(i_mem_dina), 
        .q_mem_douta(r_mem_douta)
    );

endmodule