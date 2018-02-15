// prco_core

module prco_core(
    input i_clk,
    input i_en,
    input i_reset,

    input       i_p_stalled,
    input       i_p_dec_block,

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

    wire r_cor_i_p_stalled;
    reg r_cor_i_p_valid;
    wire r_cor_q_p_stalled;
    reg r_cor_q_p_valid;
    reg r_cor_i_p_ce = 1;
    wire r_cor_q_p_ce;
    
    wire r_dec_i_p_stalled;
    wire r_dec_i_p_valid;
    wire r_dec_q_p_stalled;
    wire r_dec_q_p_valid;
    wire r_dec_i_p_ce;
    wire r_dec_q_p_ce;

    wire r_mem_i_p_stalled;
    wire r_mem_i_p_valid;
    wire r_mem_q_p_stalled;
    wire r_mem_q_p_valid;
    wire r_mem_i_p_ce;
    wire r_mem_q_p_ce;

    wire r_reg_i_p_stalled;
    wire r_reg_i_p_valid;
    wire r_reg_q_p_stalled;
    wire r_reg_q_p_valid;
    wire r_reg_i_p_ce;
    wire r_reg_q_p_ce;
    wire r_reg_q_p_cp;

    initial begin
        r_cor_i_p_valid = 1;
    end

    reg r_can_do_once = 0;

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
            r_can_do_once = 1;
        end else if (r_reg_q_p_cp) begin
            r_can_do_once <= 1;
        end else if (r_cor_q_p_ce) begin
            if (r_can_do_once) begin
                pc <= pc + 1;
                r_can_do_once <= 0;
            end
        end else begin
            r_can_do_once = 1;
        end
    end
    
    // Pipeline control
    // We are stalled if we are ready but the next stage isn't ready (stalled)
    assign r_cor_q_p_stalled = r_cor_q_p_valid && (r_cor_i_p_stalled);

    // Ready to progress if: previous stage is ready (valid)
    // and next stage isn't stalled.
    assign r_cor_q_p_ce = r_cor_i_p_valid && !r_cor_i_p_stalled;

    always @(posedge i_clk) begin
        if (i_reset || r_reg_q_p_cp) begin
            r_cor_q_p_valid <= 0;
        end else if (r_cor_q_p_ce) begin
            r_cor_q_p_valid <= r_cor_i_p_valid;
        end else if (r_cor_i_p_ce) begin
            r_cor_q_p_valid <= 0;
        end
    end

    // Instantiate the module
    prco_lmem inst_lmem (
        .i_clk(i_clk), 
        .i_reset(i_reset),

        .i_p_cp(r_reg_q_p_cp),
        .i_p_ce(r_mem_i_p_ce),
        .q_p_ce(r_mem_i_p_ce),
        .i_p_stalled(r_cor_q_p_stalled),
        .q_p_stalled(r_mem_q_p_stalled),
        .i_p_valid(r_cor_q_p_valid),
        .q_p_valid(r_mem_q_p_valid),

        .i_mem_we(i_mem_we), 
        .i_mem_addr(r_mem_addr), 
        .i_mem_dina(i_mem_dina), 
        .q_mem_douta(r_mem_douta)
    );

    // Instantiate the module
    prco_decoder inst_decoder (
        .i_clk(i_clk), 
        .i_en(r_dec_en), 

        .i_p_cp(r_reg_q_p_cp),
        .i_p_ce(r_dec_i_p_ce),
        .q_p_ce(r_dec_i_p_ce),
        .i_p_stalled(r_mem_q_p_stalled),
        .q_p_stalled(r_dec_q_p_stalled),
        .i_p_valid(r_mem_q_p_valid),
        .q_p_valid(r_dec_i_p_valid),

        .i_p_block(i_p_dec_block),

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
        .i_reset(i_reset), 

        .q_p_cp(r_reg_q_p_cp),
        .i_p_cp(r_reg_q_p_cp),
        .i_p_ce(r_reg_i_p_ce),
        .q_p_ce(r_reg_i_p_ce),
        .i_p_stalled(r_dec_q_p_stalled),
        .q_p_stalled(r_cor_i_p_stalled),
        .i_p_valid(r_dec_i_p_valid),
        .q_p_valid(r_cor_i_p_valid),

        .i_en(r_reg_en), 
        .i_sela(r_dec_seld), 
        .q_data(r_reg_doutd), 
        .i_selb(r_dec_sela), 
        .q_datb(r_reg_douta), 
        .i_we(r_dec_we), 
        .i_seld(r_dec_seld), 
        .i_datd(r_dec_imm8)
    );


endmodule