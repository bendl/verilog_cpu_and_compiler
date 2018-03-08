// prco_core

module xc6lx9_msp(
    input clk50,
    input [3:0] DIPSW,
    
    // uart tx 
    output PORTC10,
    output PORTC11,
    
    output [7:0] LEDS
);

    reg     inst_core_enable = 1;
    wire    q;

    wire    w_leds;
    
    reg r_top_reset = 1;
    wire r_top_debug_instr_clk;
    
    wire [7:0] r_top_q_debug;
    
    // uart logic
    wire [7:0] r_uart_tx_byte;

    // Instantiate the module
    prco_core inst_core (
        .i_clk(clk50), 
        .i_en(inst_core_enable), 
        .i_reset(r_top_reset),
    
        .i_rx(r_uart_i_rx),
        .q_tx(r_uart_q_tx),
        .q_tx_byte(r_uart_tx_byte),
        .q_debug_instr_clk(r_top_debug_instr_clk),
        .q_debug(r_top_q_debug)
    );
    
    always @(posedge clk50) begin
        if(r_top_reset) begin
            r_top_reset <= 0;
        end
    end
    
    assign LEDS = { {5{1'b0}}, r_uart_q_tx, clk50, r_top_debug_instr_clk};
    assign PORTC11 = r_uart_q_tx;
    assign PORTC10 = r_uart_i_rx;

endmodule