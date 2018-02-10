// prco_core

module xc6lx9_msp(
    input clk50,
    output [7:0] LEDS
);

    reg     inst_core_enable = 1;
    wire    q;

    wire    w_leds;

    // Instantiate the module
    prco_core inst_core (
        .i_clk(clk50), 
        .i_en(inst_core_enable), 
        .q_debug(LEDS[7:0])
    );

endmodule