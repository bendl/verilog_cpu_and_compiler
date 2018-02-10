// PRCO Register Set

`include "inc/prco_constants.v"

module prco_regs (
    input               i_clk,
    input               i_en,
    input               i_reset,

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
    reg [15:0] r_regs[0:7];
    
    // Reset interator
    integer i;

    always @(posedge i_clk, posedge i_reset, posedge i_en)
    begin
        if(i_en == 1) begin
            // Reset the register set
            if(i_reset == 1) begin
                $display("Resetting Registers");
                for(i = 0; i < 6; i = i + 1) begin
                    r_regs[i] <= 16'h0;
                end
                r_regs[`REG_SP] <= 16'h00FF;
                r_regs[`REG_BP] <= 16'h00FF;
            end 
            // Return values
            else begin
                // Write to a register
                if(i_we == 1) begin
                    r_regs[i_seld] <= i_datd;
                end

                q_data <= r_regs[i_sela];
                q_datb <= r_regs[i_selb];
            end
        end
    end

endmodule