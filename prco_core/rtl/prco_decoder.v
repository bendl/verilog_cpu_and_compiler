// PRCO Register Set

`include "inc/prco_constants.v"
`include "inc/prco_isa.v"

module prco_decoder (
    input i_clk,
    input i_en,
    
    input [15:0]                i_instr,

    output reg [4:0]            q_op,
    output reg [2:0]            q_seld,
    output reg [2:0]            q_sela,
    output reg unsigned [7:0]   q_imm8,
    output reg signed [4:0]     q_simm5,

    output reg                  q_reg_we
);

    /// Task to set appropriate output signals for the
    /// incoming i_instr[15:0]
    task handle_opcode;
        input [4:0] ti_op;

        case (ti_op)
            `PRCO_OP_NOP: begin
                $display("PRCO_OP_NOP");
                q_sela <= i_instr[7:5];
                q_reg_we <= 0;
                end
            `PRCO_OP_MOVI: begin
                $display("PRCO_OP_MOVI\t%d, %d", q_imm8, q_seld);
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                end
            `PRCO_OP_MOV: begin
                $display("PRCO_OP_MOV\t%d, %d", q_sela, q_seld);
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                end
            `PRCO_OP_ADD: begin
                $display("PRCO_OP_ADD\t%d, %d", q_sela, q_seld);
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                end

            default: begin
                $display("Unknown op: %h", ti_op);
                q_sela <= i_instr[7:5];
                q_reg_we <= 0;
                end
        endcase
    endtask

    // TODO(ben): Should sensitivity list include i_instr?
    always @(posedge i_clk, posedge i_en, i_instr) begin
        if(i_en == 1) begin
            q_op <= i_instr[15:11];
            q_seld <= i_instr[10:8];
            q_imm8 <= i_instr[7:0];
            q_simm5 <= i_instr[4:0];
            
            // Decode opcode and set outputs
            handle_opcode(i_instr[15:11]);
        end
    end

endmodule
