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

    // Output 5 bit opcode
    output reg [4:0]            q_op,
    // Output Rd and Ra register selects
    output reg [2:0]            q_seld,
    output reg [2:0]            q_sela,

    // Output identifying a type 3 instruction (3 reg selects)
    output reg                  q_third_sel,
    output reg [2:0]            q_selb,

    // Output immediate values
    output reg unsigned [7:0]   q_imm8,
    output reg signed [4:0]     q_simm5,

    // Output dependencies
    output reg                  q_reg_we,
    output reg                  q_req_alu,
    output reg                  q_req_ram,
    output reg                  q_req_ram_we,
    output reg                  q_new_uart1_data,
    output reg                  q_halt
);

    /// Task to set appropriate output signals for the
    /// incoming i_instr[15:0]
    task handle_opcode;
        input [4:0] ti_op;

        case (ti_op)
            `PRCO_OP_NOP: begin
                q_sela <= i_instr[7:5];
                q_third_sel     <= 0;

                q_reg_we <= 0;
                q_req_ram <= 0;
                q_fetch <= 1;
                q_ce <= 0;
                q_req_ram_we <= 0;
                q_new_uart1_data <= 0;
                q_halt <= 0;
                $display("PRCO_OP_NOP");
                end
                
            `PRCO_OP_MOVI: begin
                q_sela <= i_instr[7:5];
                q_third_sel     <= 0;

                q_reg_we <= 1;
                q_req_ram <= 0;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 0;
                q_new_uart1_data <= 0;
                q_halt <= 0;
                $display("PRCO_OP_MOVI\ti%h, d%h", i_instr[7:0], i_instr[10:8]);
                end
                
            `PRCO_OP_MOV: begin
                q_sela <= i_instr[7:5];
                q_third_sel     <= 0;
                
                q_reg_we <= 1;
                q_req_ram <= 0;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 0;
                q_new_uart1_data <= 0;
                q_halt <= 0;
                $display("PRCO_OP_MOV\ta%h, d%h", i_instr[7:5], i_instr[10:8]);
                end
                
            `PRCO_OP_ADD: begin
                q_sela <= i_instr[7:5];
                q_third_sel     <= 0;

                q_reg_we <= 1;
                q_req_ram <= 0;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 0;
                q_new_uart1_data <= 0;
                q_halt <= 0;
                $display("PRCO_OP_ADD\ta%h, d%h", i_instr[7:5], i_instr[10:8]);
                end

            `PRCO_OP_SUBI,
            `PRCO_OP_ADDI: begin
                q_sela <= i_instr[7:5];
                q_third_sel     <= 0;

                q_reg_we <= 1;
                q_req_ram <= 0;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 0;
                q_new_uart1_data <= 0;
                q_halt <= 0;
                $display("PRCO_OP_ATHI\t%h, %h", q_imm8, i_instr[10:8]);
                end

            `PRCO_OP_LW: begin
                q_sela <= i_instr[7:5];
                q_third_sel     <= 0;

                q_reg_we <= 1;
                q_req_ram <= 1;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 0;
                q_new_uart1_data <= 0;
                q_halt <= 0;
                $display("PRCO_OP_LW\td%h, o%h(a%h)", 
                    i_instr[10:8], i_instr[4:0], i_instr[7:5]);
                end

            `PRCO_OP_SW: begin
                q_sela <= i_instr[7:5];
                q_third_sel     <= 0;

                q_reg_we <= 0;
                q_req_ram <= 1;
                q_fetch <= 0;
                q_ce <= 1;
                q_req_ram_we <= 1;
                q_new_uart1_data <= 0;
                q_halt <= 0;
                $display("PRCO_OP_SW\t%h, %h", i_instr[10:8], q_sela);
                end

            `PRCO_OP_CMP: begin
                $display("PRCO_OP_CMP\t%d, %d, %d", i_instr[10:8], q_sela, q_selb);

                q_sela          <= i_instr[7:5];
                q_third_sel     <= 0;
                
                q_reg_we        <= 0;
                q_req_ram       <= 0;
                q_ce            <= 1;
                q_req_ram_we    <= 0;
                q_new_uart1_data <= 0;
                q_halt <= 0;
                end
            
            `PRCO_OP_JMP: begin
                $display("PRCO_OP_JMP\t%d %d", i_instr[10:8], q_imm8);
                q_sela          <= i_instr[7:5];
                q_third_sel     <= 0;

                q_reg_we        <= 1;
                q_req_ram       <= 0;
                q_ce            <= 1;
                q_req_ram_we    <= 0;
                q_new_uart1_data <= 0;
                q_halt <= 0;
                end
            
            `PRCO_OP_WRITE: begin
                $display("PRCO_OP_WRITE\t%d %d", q_seld, q_imm8);
                q_sela          <= i_instr[7:5];
                q_third_sel     <= 0;

                q_reg_we        <= 0;
                q_req_ram       <= 0;
                q_ce            <= 1;
                q_req_ram_we    <= 0;
                q_new_uart1_data <= 1;
                q_halt <= 0;
                end

            default: begin
                q_sela <= i_instr[7:5];
                q_third_sel     <= 0;
                
                q_reg_we <= 0;
                q_req_ram <= 0;
                q_fetch <= 1;
                q_ce <= 0;
                q_req_ram_we <= 0;
                q_new_uart1_data <= 0;
                q_halt <= 1;
                $display("Unknown op: %h", ti_op);
                $display("HALTING!!!");
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
