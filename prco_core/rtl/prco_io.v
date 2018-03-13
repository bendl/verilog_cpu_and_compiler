/*

*/

module prco_io (
    input i_clk,

    input i_new_data_uart1,
    input [7:0]     i_8bit_data,

    output reg [7:0] q_UART1_tx_data,

    output reg q_GPIO1,
    output reg q_GPIO2
    );

    initial begin
        q_UART1_tx_data <= 8'h0;
    end

    always @(posedge i_clk) begin
        if(i_new_data_uart1) begin
            $display("New data on input: %h", i_8bit_data);
            q_UART1_tx_data <= i_8bit_data;
        end
    end

endmodule