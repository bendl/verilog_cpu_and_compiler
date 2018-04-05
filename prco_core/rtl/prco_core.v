// prco_core

`include "inc/prco_constants.v"
`include "inc/prco_misc.v"

module prco_core(
    input           i_clk,
    input           i_en,
    input           i_reset,

    // Debug control
    input           i_mode,
    input           i_step,
    
    input           i_rx,
    output          q_tx,
    output [7:0]    q_tx_byte,
    
    output reg      q_debug_instr_clk,
    output [7:0]    q_debug
);

    // program counter
    reg [15:0]  pc = 0;
    reg         pc_branch = 0;
    
    reg [3:0] core_state = 4'b0;

    // Debug out port
    assign q_debug[7:0] = {
        {1{1'b0}},      // 4
        pc[6:0]         // 4
    };

    wire [5:0]  r_dec_op;
    wire [2:0]  r_dec_seld;
    wire [2:0]  r_dec_sela;
    wire        r_dec_we;
    wire [15:0] r_dec_imm8;
    wire [4:0]  r_dec_simm5;

    wire [15:0] r_reg_doutd;
    wire [15:0] r_reg_douta;
    reg         r_reg_en = 1;
    reg [15:0]  r_reg_dina;

    // Tempory registers for storing register select paths
    // for switching between type 2 and 3 instructions.
    reg [2:0]   r_reg_sela;
    reg [2:0]   r_reg_selb;
    
    reg         r_dec_en = 1;

    // Only write to a register when the decode instr requires it
    // and: 
    //   - We are at the write-back stage
    wire        r_reg_we = r_dec_we & (r_reg_q_ce_fetch);
    wire        r_lmem_we = r_dec_req_ram_we & (r_mem_i_ce);
    wire        r_io_i_new_data_uart = r_dec_q_new_uart1_data & (r_reg_q_ce_fetch);
    
    wire [15:0] r_alu_result;
    wire [15:0] r_mem_douta;

    reg [15:0]  r_mem_addr;

    // UART logic
    wire        uart_tx;
    wire        uart_reset = i_reset;
    
    assign      q_tx =      uart_tx;
    assign      q_tx_byte = 8'h0;

    reg         uart_can_tx = 0;
    wire        uart_transmit = r_dec_q_new_uart1_data & r_reg_q_ce_fetch & !uart_tx_fifo_full;
    reg         uart_rx_fifo_pop = 0;
    // TODO: convert input to 50mhz
    wire        uart_clk50 = i_clk;
    wire [7:0]  uart_rx_byte;
    wire        uart_irq;
    wire        uart_busy;
    wire        uart_tx_fifo_full;
    wire        uart_rx_fifo_empty;
    wire        uart_is_transmitting;

    // Offset the ALU -> RAM request by 1 clock cycle
    reg         r_mem_int_i_ce = 0;
    reg         r_mem_i_ce = 0;

    // Pipeline signals
    // Jump-start the CPU into running
    reg           r_cpu_init;
    reg           i_ce_reg;
    wire          i_ce = r_cpu_init || (r_reg_q_ce_fetch || r_dec_q_fetch);
    reg           q_ce;
    
    always @(posedge i_clk) begin
        if(i_ce) begin
        end
    end
    

    always @(posedge i_clk) begin
        if(r_alu_q_ce_ram) begin
            r_mem_addr <= r_alu_result;
            r_mem_int_i_ce <= 1;
        end else if (r_mem_int_i_ce) begin
            r_mem_i_ce <= 1;
            r_mem_int_i_ce <= 0;
        end else begin
            r_mem_int_i_ce <= 0;
            r_mem_i_ce <= 0;
            r_mem_addr <= pc;
        end
    end

    always @(posedge i_clk) begin
        if(r_mem_q_ce_reg) begin
            r_reg_dina <= r_mem_douta;
        end else begin
            r_reg_dina <= r_alu_result;
        end
    end

    always @(posedge i_clk, posedge i_reset) begin
        if (i_reset == 1) begin
            pc <= 0;
            q_ce <= 0;
            r_cpu_init <= 1;
        end else begin
            `PULSE_SIGNAL(q_ce);
            r_cpu_init <= 0;
            
            if(r_alu_q_should_branch) begin
                core_state <= 1;
            end
            if(q_debug_instr_clk) begin
                q_debug_instr_clk <= 0;
            end
            
            if(i_ce) begin
                i_ce_reg <= 1;
            end
            
            if(r_dec_q_halt) begin
                // Halt, do nothing...
                q_ce <= 0;
            end 
            // Normal PC increment
            else if (core_state == 0) begin
                // Pipeline telling us that we can start
                // next instruction
                if(i_ce_reg) begin
                    // If we are in debug mode, don't
                    // automatically increment pc, etc.
                    // 
                    // We must wait for the i_step signal
                    if((i_mode == 0) || (i_mode == 1 && i_step)) 
                    begin                    
                        q_debug_instr_clk <= 1;
                        pc <= pc + 1;
                        
                        $display("PC 0x%x", pc);
                        
                        // We have acted upon the data and
                        // are ready to pass to next module
                        q_ce <= 1;
                        i_ce_reg <= 0;
                    end
                end else begin
                    q_ce <= 0;
                end
            end
            // If jump required
            // TODO: core_state is a disgusting fix, fix it...
            else if (core_state == 1) begin
                $display("core state == 1");
                pc <= r_alu_result;
                core_state <= core_state + 1;
            end else if(core_state == 4'b0010) begin
                core_state <= core_state + 1;
            end else if(core_state == 4'b0011) begin
                core_state <= core_state + 1;
            end else if(core_state == 4'b100) begin
                q_ce <= 1;
                core_state <= 0;
            end
        end
    end


    // Instantiate the module
    prco_lmem inst_lmem (
        .i_clk(i_clk), 

        .i_ce_fetch(q_ce),
        .i_ce_alu(r_mem_i_ce),

        .q_ce_dec(r_mem_q_ce_decode),
        .q_ce_reg(r_mem_q_ce_reg),
        
        .i_mem_we(r_lmem_we), 
        .i_mem_addr(r_mem_addr), 
        .i_mem_dina(r_reg_doutd), 
        .q_mem_douta(r_mem_douta)
    );

    // Instantiate the module
    prco_decoder inst_decoder (
        .i_clk(i_clk), 
        .i_en(r_dec_en), 

        .i_ce(r_mem_q_ce_decode),
        .q_ce(r_dec_q_ce),
        .q_fetch(r_dec_q_fetch),

        .i_instr(r_mem_douta), 
        .q_op(r_dec_op), 

        .q_seld(r_dec_seld), 
        .q_sela(r_dec_sela),
        .q_selb(r_dec_selb),
        .q_third_sel(r_dec_q_third_sel),

        .q_imm8(r_dec_imm8),
        .q_simm5(r_dec_simm5),
        .q_reg_we(r_dec_we),

        .q_req_ram(r_dec_ram_req),
        .q_req_ram_we(r_dec_req_ram_we),
        .q_new_uart1_data(r_dec_q_new_uart1_data),
        .q_halt(r_dec_q_halt)
    );

    // Instantiate the module
    prco_regs inst_regs (
        .i_clk(i_clk), 
        .i_en(r_reg_en), 

        .i_ce_ram   (r_mem_q_ce_reg),
        .i_ce_dec   (r_dec_q_ce),
        .i_ce_alu   (r_alu_q_ce_reg),
        .q_ce_alu   (r_reg_q_ce_alu),
        .q_ce_fetch (r_reg_q_ce_fetch),

        .i_reset(i_reset), 

        .i_sela(r_dec_seld), 
        .i_selb(r_dec_sela), 

        .q_data(r_reg_doutd), 
        .q_datb(r_reg_douta), 

        .i_we(r_reg_we), 
        .i_seld(r_dec_seld), 
        .i_datd(r_reg_dina)
    );
    
    // Instantiate the module
    prco_alu inst_alu (
        .i_clk(i_clk), 

        .i_ce(r_reg_q_ce_alu),
        .i_dec_req_ram(r_dec_ram_req),
        .q_ce_ram(r_alu_q_ce_ram),
        .q_ce_reg(r_alu_q_ce_reg),

        .i_op(r_dec_op), 

        .i_data(r_reg_douta), 
        .i_datb(r_reg_doutd), 

        .i_imm8(r_dec_imm8), 
        .i_simm5(r_dec_simm5), 
        .q_result(r_alu_result),
        .q_should_branch(r_alu_q_should_branch)
    );
    
    prco_io inst_io (
        .i_clk(i_clk),
        .i_new_data_uart1(r_io_i_new_data_uart),
        .i_8bit_data(r_alu_result[7:0]),
        .q_UART1_tx_data(r_io_q_UART1_tx_data),
        .q_GPIO1(r_io_q_GPIO1)
    );

    reg [7:0] r_io_test = 8'hEE;
    uart_fifo uart_fifo(
        // Outputs
        .rx_byte         (uart_rx_byte[7:0]),
        .tx              (uart_tx),
        .irq             (uart_irq),
        .busy            (uart_busy),
        .tx_fifo_full    (uart_tx_fifo_full),
        .rx_fifo_empty   (uart_rx_fifo_empty),
        //.is_transmitting (is_transmitting),
        // Inputs
        .tx_byte         (r_alu_result[7:0]),
        .clk             (uart_clk50),
        .rst             (uart_reset),
        .rx              (uart_rx),
        .transmit        (uart_transmit),
        .rx_fifo_pop     (uart_rx_fifo_pop)
    );

endmodule
