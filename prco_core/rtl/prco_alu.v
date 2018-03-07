// prco_alu.v

`include "inc/prco_constants.v"
`include "inc/prco_isa.v"
`include "inc/prco_debug.v"

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

    output reg signed [15:0] q_result,
    output reg               q_should_branch
);
    reg [15:0] sign_extended_imm;

    // Set the CMP bits
    function [15:0] func_alu_cmp;
        input [15:0] data;
        input [15:0] datb;
        reg [16:0] tmp;
        reg [15:0] ret = 0;
        
        begin
            tmp = data - datb;
            $display(`BINP17, `BIN17(tmp));

            // Zero flag (or Equal)
            if(tmp == 0) ret[`SR_Z] = 1;

            // Signed flag
            ret[`SR_S] = tmp[15];
            
            // Overflow flag
            // https://stackoverflow.com/questions/30957188/
            case(tmp[15:14])
                2'b01: ret[`SR_O] = 1;
                2'b10: ret[`SR_O] = 1;
                default: ret[`SR_O] = 0;
            endcase
            
            // Return the set info bits
            $display(`BINP16, `BIN16(ret));
            func_alu_cmp = ret;
        end
    endfunction;
    
    // Decide if a JMP should be performed based on input bits
    // and the SR register (`REG_SR)
    function [0:0] func_alu_should_jmp;
        input [7:0] instr_flags;
        input [15:0] sr_flags;
        begin
            $display("JMP OP: %d", instr_flags);
            case(instr_flags)
                `PRCO_OP_JMP_J: begin
                    // Unconditional jump
                    $display("JMP Unconditional");
                    func_alu_should_jmp = 1;
                    end
                `PRCO_OP_JMP_JE: begin
                    // Z=1
                    func_alu_should_jmp = (sr_flags[`SR_Z] == 1);
                    end
                `PRCO_OP_JMP_JNE: begin
                    // Z=0
                    func_alu_should_jmp = (sr_flags[`SR_Z] == 0);
                    end
                `PRCO_OP_JMP_JS: begin
                    // S=1
                    func_alu_should_jmp = (sr_flags[`SR_S] == 1);
                    end
                `PRCO_OP_JMP_JNS: begin
                    // S=0
                    func_alu_should_jmp = (sr_flags[`SR_S] == 0);
                    end
                `PRCO_OP_JMP_JG: begin
                    // Z=0, S=O
                    func_alu_should_jmp = (sr_flags[`SR_Z] == 0 & 
                        (sr_flags[`SR_S] == sr_flags[`SR_O]));
                    end
                `PRCO_OP_JMP_JGE: begin
                    // S=O
                    func_alu_should_jmp = (sr_flags[`SR_S] == sr_flags[`SR_O]);
                    end
                `PRCO_OP_JMP_JL: begin
                    // S<>0
                    func_alu_should_jmp = (sr_flags[`SR_S] != sr_flags[`SR_O]);
                    end
                `PRCO_OP_JMP_JLE: begin
                    // Z=1 or S<>O
                    func_alu_should_jmp = (sr_flags[`SR_Z] | 
                        (sr_flags[`SR_S] != sr_flags[`SR_O]));
                    end
                default: begin
                    $display("Unknown JMP OP %d", instr_flags);
                    func_alu_should_jmp = 0;
                    end
            endcase;
        end
    endfunction;

    always @(posedge i_clk) begin
      if (i_ce) begin
        case (i_op)
        `PRCO_OP_SW,
        `PRCO_OP_LW: begin
                $display("ALU_PRCO_OP_S/LW");
                // Sign extend the signed 5 bit immediate
                sign_extended_imm = { {11{i_simm5[4]}}, i_simm5[4:0] };
                q_result <= i_data + sign_extended_imm;
                q_should_branch <= 0;
            end

        `PRCO_OP_CMP: begin
            $display("ALU_PRCO_OP_CMP: Comparing %d to %d",
                i_data, i_datb);
            q_result <= func_alu_cmp(i_data, i_datb);
            q_should_branch <= 0;
            end

        `PRCO_OP_JMP: begin
            // ALU result is the PC in address
            // datb = register to jump to
            // data = SR register
            q_result <= i_datb;
            q_should_branch <= func_alu_should_jmp(i_imm8, i_data);
            end

        `PRCO_OP_MOVI: begin
            q_result[15:8] <= 0;
            q_result[7:0] <= i_imm8;
            q_should_branch <= 0;
            end

        `PRCO_OP_MOV: begin
            q_result <= i_datb;
            q_should_branch <= 0;
            end

        `PRCO_OP_ADD: begin
            q_result <= i_data + i_datb;
            q_should_branch <= 0;
            end

        `PRCO_OP_ADDI: begin
            q_result <= i_datb + i_imm8;
            q_should_branch <= 0;
            end

        `PRCO_OP_SUBI: begin
            q_result <= i_datb - i_imm8;
            q_should_branch <= 0;
            end

        `PRCO_OP_OR: begin
            q_result <= i_data | i_datb;
            q_should_branch <= 0;
            end
        `PRCO_OP_XOR: begin
            q_result <= i_data ^ i_datb;
            q_should_branch <= 0;
            end
        `PRCO_OP_AND: begin
            q_result <= i_data & i_datb;
            q_should_branch <= 0;
            end

        `PRCO_OP_NOP: begin
            q_result <= 16'h0000;
            end

        default: begin
            $display("ALU: Unknown op: %h", i_op);
            q_result <= 16'h0000;
            q_should_branch <= 0;
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

      if(q_ce_ram || q_ce_reg || q_should_branch) begin
          q_ce_ram <= 0;
          q_ce_reg <= 0;
          q_should_branch <= 0;
      end
    end

endmodule