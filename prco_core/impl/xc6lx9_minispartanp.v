// prco_core

module xc6lx9_msp(
    input clk50,
    output [7:0] LEDS
);

    reg     inst_core_enable = 1;
    wire    q;

    wire    w_leds;
    
    reg r_top_reset = 1;
    wire r_top_debug_instr_clk;
    
    wire [7:0] r_top_q_debug;

    // Instantiate the module
    prco_core inst_core (
        .i_clk(clk50), 
        .i_en(inst_core_enable), 
        .i_reset(r_top_reset),
        .q_debug_instr_clk(r_top_debug_instr_clk),
        .q_debug(r_top_q_debug)
    );
    
    always @(posedge clk50) begin
        if(r_top_reset) begin
            r_top_reset <= 0;
        end
    end
    
    assign LEDS = { {6{1'b0}}, clk50, r_top_debug_instr_clk};

endmodule