// prco_alu.v

`include "inc/prco_constants.v"
`include "inc/prco_isa.v"

module prco_alu (
    input i_clk,

    // Pipeline signals
    input           i_ce,
    input           i_dec_req_ram,
    output reg      q_ce_reg,
    output reg      q_ce_ram,

    input [4:0]     i_op,
    input signed [15:0] i_data,
    input signed [15:0] i_datb,
    input signed [7:0] i_imm8,
    input signed [4:0] i_simm5,

    output reg signed [15:0] q_result
);
    reg [15:0] sign_extended_imm;

    always @(posedge i_clk) begin
      if (i_ce) begin
        case (i_op)
        `PRCO_OP_SW,
        `PRCO_OP_LW: begin
                $display("ALU_PRCO_OP_S/LW");
                // Sign extend the signed 5 bit immediate
                sign_extended_imm = { {11{i_simm5[4]}}, i_simm5[4:0] };
                q_result = i_data + sign_extended_imm;
                // i_data
            end

        `PRCO_OP_MOVI: begin
            q_result[15:8] <= 0;
            q_result[7:0] <= i_imm8;
            end

        `PRCO_OP_MOV: begin
            q_result <= i_datb;
            end

        `PRCO_OP_ADD: begin
            q_result <= i_data + i_datb;
            end

        `PRCO_OP_NOP: begin
            q_result <= 16'h0000;
            end

        default: begin
            $display("ALU: Unknown op: %h", i_op);
            q_result <= 16'h0000;
            end
        endcase

        if(i_dec_req_ram) begin
            q_ce_ram <= 1;
            q_ce_reg <= 0;
        end else begin
            q_ce_ram <= 0;
            q_ce_reg <= 1;
        end
      end

      if(q_ce_ram || q_ce_reg) begin
          q_ce_ram <= 0;
          q_ce_reg <= 0;
      end
    end

endmodule