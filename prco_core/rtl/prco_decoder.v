// PRCO Register Set

`include "inc/prco_constants.v"
`include "inc/prco_isa.v"

module prco_decoder (
    input i_clk,
    input i_reset,
    
    input i_en,
    
    // Pipeline control
    input               i_p_cp,
    input               i_p_stalled,
    input               i_p_valid,
    output              q_p_stalled,
    output reg          q_p_valid,
    input               i_p_ce,
    output              q_p_ce,

    input               i_p_block,
    
    input [15:0]                i_instr,

    output reg [4:0]            q_op,
    output reg [2:0]            q_seld,
    output reg [2:0]            q_sela,
    output reg unsigned [7:0]   q_imm8,
    output reg signed [4:0]     q_simm5,

    output reg                  q_reg_we,
    output reg                  q_req_alu,
    output reg                  q_req_ram
);
    reg [15:0] b_instr;

    /// Task to set appropriate output signals for the
    /// incoming i_instr[15:0]
    task handle_opcode;
        input [4:0] ti_op;

        case (ti_op)
            `PRCO_OP_NOP: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 0;
                $display("PRCO_OP_NOP");
                end
                
            `PRCO_OP_MOVI: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                $display("PRCO_OP_MOVI\t%d, %d", q_imm8, q_seld);
                end
                
            `PRCO_OP_MOV: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                $display("PRCO_OP_MOV\t%d, %d", q_sela, q_seld);
                end
                
            `PRCO_OP_ADD: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 1;
                $display("PRCO_OP_ADD\t%d, %d", q_sela, q_seld);
                end

            default: begin
                q_sela <= i_instr[7:5];
                q_reg_we <= 0;
                $display("Unknown op: %h", ti_op);
                end
        endcase
    endtask

    // TODO(ben): Should sensitivity list include i_instr?
    always @(posedge i_clk, posedge i_en) begin
        if(i_en == 1) begin
            
        end
    end

    // Pipeline control
    // We are stalled if we are ready but the next stage isn't ready (stalled)
    assign q_p_stalled = q_p_valid && (i_p_stalled || i_p_block);

    // Ready to progress if: previous stage is ready (valid)
    // and next stage isn't stalled.
    assign q_p_ce = i_p_valid && !q_p_stalled;

    always @(posedge i_clk) begin
        if (q_p_stalled) begin
            $display("DEC: Stalled because of mem!");
        end

        if (q_p_ce) begin
            $display("DEC: doing...");
        end
    end

    always @(posedge i_clk) begin
        if (i_reset || i_p_cp) begin
            q_p_valid <= 0;
        end else if (q_p_ce) begin
            q_p_valid <= i_p_valid;

            b_instr <= i_instr;
            q_op <= i_instr[15:11];
            q_seld <= i_instr[10:8];
            q_imm8 <= i_instr[7:0];
            q_simm5 <= i_instr[4:0];
            
            // Decode opcode and set outputs
            handle_opcode(i_instr[15:11]);

        end else if (i_p_ce) begin
            q_p_valid <= 0;
        end
    end

endmodule
