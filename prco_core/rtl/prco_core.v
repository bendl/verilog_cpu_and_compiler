// prco_core

module prco_core(
    input i_clk,
    input i_en,

    output [7:0] q_debug,
    output q
);

    assign q = i_clk;

    assign q_debug[0] = i_clk;

endmodule