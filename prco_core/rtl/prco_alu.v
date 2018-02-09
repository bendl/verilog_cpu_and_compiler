// PRCO Register Set

module prco_regs(
    input i_clk,
    input i_en,
    input i_reset
);

    reg [15:0] r_gregs [7:0];

    always @(posedge i_clk, posedge i_rset, posedge i_en)
    begin
        
    end

endmodule