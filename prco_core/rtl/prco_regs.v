// PRCO Register Set

`include "inc/prco_constants.v"

module prco_regs (
    input               i_clk,
    input               i_en,
    input               i_reset,

    // Pipeline signals
    input           i_ce_ram,
    input           i_ce_dec,
    input           i_ce_alu,
    output reg      q_ce_fetch,
    output reg      q_ce_alu,

    // Dual port memory access
    input [2:0]         i_sela,
    output reg [15:0]   q_data,
    input [2:0]         i_selb,
    output reg [15:0]   q_datb,

    // Write enable pin
    input               i_we,
    input [2:0]         i_seld,
    input [15:0]        i_datd
);
    // 8 16-bit registers
    reg [15:0] r_regs[0:7]/*verilator public_flat*/;

    reg [7:0] foo /*verilator public_flat*/;
    
    // Reset interator
    integer i;

    always @(posedge i_clk, posedge i_reset, posedge i_en)
    begin
        // Reset the register set
        if(i_reset == 1) begin
            $display("Resetting Registers");
            for(i = 0; i < 6; i = i + 1) begin
                r_regs[i] <= 16'h0;
            end
            r_regs[`REG_SP] <= 16'h00FF;
            r_regs[`REG_BP] <= 16'h00FF;

            // Output something so we don't latch
            q_data <= 16'h0;
            q_datb <= 16'h0;

            q_ce_alu <= 0;
            q_ce_fetch <= 1;
        end else if(i_ce_dec || i_ce_ram || i_ce_alu || i_we) begin
            // Write to a register
            if(i_we == 1) begin
                $display("Writing register %d value: %h",
                    i_seld, i_datd);
                r_regs[i_seld] <= i_datd;
            end

            // Write output
            q_data <= r_regs[i_sela];
            q_datb <= r_regs[i_selb];

            if (i_ce_alu) begin
                q_ce_alu <= 0;
                q_ce_fetch <= 1;
            end else if(i_ce_dec) begin
                q_ce_alu <= 1;
                q_ce_fetch <= 0;
            end else if (i_ce_ram) begin
                q_ce_alu <= 0;
                q_ce_fetch <= 1;
            end else begin
                q_ce_alu <= 0;
                q_ce_fetch <= 0;
            end
        end else begin
            q_ce_alu <= 0;
            q_ce_fetch <= 0;
        end

        if(q_ce_alu || q_ce_fetch) begin
            q_ce_alu <= 0;
            q_ce_fetch <= 0;
        end
    end

endmodule