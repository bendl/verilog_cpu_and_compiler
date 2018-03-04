// PRCO Register Set

`include "inc/prco_constants.v"
`include "inc/prco_isa.v"

module prco_decoder (
    input i_clk,
    input i_reset,
    
    input i_en,

    // Pipeline signals
    input           i_ce,
    output reg      q_ce,
    output reg      q_fetch,
    
    input [15:0]                i_instr,

    output reg [4:0]            q_op,
    output reg [2:0]            q_seld,
    output reg [2:0]            q_sela,
    output reg unsigned [7:0]   q_imm8,
    output reg signed [4:0]     q_simm5,

    output reg                  q_reg_we,
    output reg                  q_req_alu,
    output reg                  q_req_ram,
    output reg                  q_req_ram_we
);

    /// Task to set appropriate output signals for the
    /// incoming i_instr[15:0]
    task handle_opcode;
        input [4:0] ti_op;

        case (ti_op)
            `PRCO_OP_NOP: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 0;
                q_req_ram <= 0;
                q_fetch <= 1;
                q_ce <= 0;
                q_req_ram_we <= 0;
                $display("PRCO_OP_NOP");
                end
                
            `PRCO_OP_MOVI: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                q_req_ram <= 0;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 0;
                $display("PRCO_OP_MOVI\t%h, %h", q_imm8, q_seld);
                end
                
            `PRCO_OP_MOV: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                q_req_ram <= 0;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 0;
                $display("PRCO_OP_MOV\t%h, %h", q_sela, q_seld);
                end
                
            `PRCO_OP_ADD: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                q_req_ram <= 0;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 0;
                $display("PRCO_OP_ADD\t%h, %h", q_sela, q_seld);
                end

            `PRCO_OP_LW: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                q_req_ram <= 1;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 0;
                $display("PRCO_OP_LW\t%h, %h(%h)", 
                    q_seld, q_simm5, q_sela);
                end

            `PRCO_OP_SW: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 0;
                q_req_ram <= 1;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 1;
                $display("PRCO_OP_SW\t%h, %h", q_seld, q_sela);
                end

            default: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 0;
                q_req_ram <= 0;
                q_fetch <= 1;
                q_ce <= 0;
                q_req_ram_we <= 0;
                $display("Unknown op: %h", ti_op);
                end
        endcase
    endtask

    // TODO(ben): Should sensitivity list include i_instr?
    always @(posedge i_clk) begin
        if(i_ce) begin
            q_op = i_instr[15:11];
            q_seld = i_instr[10:8];
            q_imm8 = i_instr[7:0];
            q_simm5 = i_instr[4:0];
            
            // Decode opcode and set outputs
            handle_opcode(i_instr[15:11]);
            
        end else begin
            q_fetch <= 0;
            q_ce <= 0;
        end

        if(q_ce) q_ce <= 0;
        if(q_fetch) q_fetch <= 0;
    end

endmodule
