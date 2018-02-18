// prco_core

`include "inc/prco_constants.v"

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
    wire [4:0]  r_dec_simm5;
    wire        r_dec_ram_req;

    wire [15:0] r_reg_doutd;
    wire [15:0] r_reg_douta;
    reg         r_reg_en = 1;
    reg [15:0]  r_reg_dina;
    wire        r_reg_we = r_dec_we & r_state_fetch;

    wire [15:0] r_alu_result;

    reg [15:0]  r_mem_addr;
    always @(posedge i_clk) begin
        if (states[0] == 1) begin
            r_mem_addr = pc;
        end else begin
            r_mem_addr = 0;
        end
    end

    always @(posedge i_clk) begin
        if (r_state_ram) r_reg_dina <= r_mem_douta;
        else r_reg_dina <= r_alu_result;
    end
    
    wire [15:0] r_mem_douta;

    reg [7:0] r_state;
    wire r_state_reset = r_state[0];
    wire r_state_fetch = r_state[1];
    wire r_state_decode = r_state[2];
    wire r_state_read = r_state[3];
    wire r_state_exec = r_state[4];
    wire r_state_ram = r_state[5];
    wire r_state_write = r_state[6];
    wire r_state_halt = r_state[7];

    wire r_reg_ce = r_state_reset || r_state_read || r_state_fetch;
    wire r_mem_ce = r_state_fetch | r_state_ram;

    // state machine
    always @(posedge(i_clk)) begin
        case (r_state)
        `STATE_RESET:
            r_state <= `STATE_FETCH;

        `STATE_FETCH:
            r_state <= `STATE_DECODE;

        `STATE_DECODE:
            r_state <= `STATE_READ;

        `STATE_READ:
            r_state <= `STATE_EXEC;

        `STATE_EXEC: begin
            pc <= pc + 1;

            if (r_dec_ram_req) begin
                r_state <= `STATE_RAM;
            end else begin
                r_state <= `STATE_WRITE;
            end
            end
        
        `STATE_RAM:
            r_state <= `STATE_WRITE;

        `STATE_WRITE:
            r_state <= `STATE_FETCH;

        default:
            r_state <= `STATE_RESET;
        endcase
    end

    always @(posedge i_clk, posedge i_reset) begin
        if (i_reset == 1) begin
            pc <= 0;
            states = 6'h1;
        end else begin
            
        end
    end    

    // Instantiate the module
    prco_lmem inst_lmem (
        .i_clk(i_clk), 

        .i_ce(r_mem_ce),
        
        .i_mem_we(i_mem_we), 
        .i_mem_addr(r_mem_addr), 
        .i_mem_dina(i_mem_dina), 
        .q_mem_douta(r_mem_douta)
    );

    // Instantiate the module
    prco_decoder inst_decoder (
        .i_clk(i_clk), 
        .i_en(r_dec_en), 

        .i_ce(r_state_decode),

        .i_instr(r_mem_douta), 
        .q_op(r_dec_op), 
        .q_seld(r_dec_seld), 
        .q_sela(r_dec_sela),
        .q_imm8(r_dec_imm8),
        .q_simm5(r_dec_simm5),
        .q_reg_we(r_dec_we),

        .q_req_ram(r_dec_ram_req)
        );

    // Instantiate the module
    prco_regs inst_regs (
        .i_clk(i_clk), 
        .i_en(r_reg_en), 

        .i_ce(r_reg_ce),

        .i_reset(i_reset), 
        .i_sela(r_dec_seld), 
        .q_data(r_reg_doutd), 
        .i_selb(r_dec_sela), 
        .q_datb(r_reg_douta), 
        .i_we(r_reg_we), 
        .i_seld(r_dec_seld), 
        .i_datd(r_reg_dina)
        );
    
    // Instantiate the module
    prco_alu inst_alu (
        .i_clk(i_clk), 

        .i_ce(r_state_exec),

        .i_op(r_dec_op), 
        .i_data(r_reg_douta), 
        .i_datb(r_reg_doutd), 
        .i_imm8(r_dec_imm8), 
        .i_simm5(r_dec_simm5), 
        .q_result(r_alu_result)
    );




endmodule