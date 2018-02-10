/*
 * Copyright (c)
 */

#include <stdio.h>
#include "verilated.h"
#include "obj_dir/prco_regs.h"
#include "obj_dir/prco_regs__Dpi.h"

struct test_item {
    int i_sela;
    int q_data_exp;

    int i_selb;
    int q_datb_exp;

    int i_seld;
    int i_we;
    int i_datd;
};

int main(int argc, char** argv) {
    prco_regs *uut = new prco_regs;

    Verilated::commandArgs(argc, argv);

    printf("Runing prco_regs test...\r\n");

    uut->i_clk = 0;
    uut->i_en = 1;
    uut->i_reset = 1;
    uut->i_sela = 0;
    uut->i_selb = 0;
    uut->i_we = 0;
    uut->i_seld = 0;
    uut->i_datd = 0;
    uut->eval();

    uut->i_clk      = 1;
    uut->eval();

    printf("%02x\r\n", uut->q_data);
    printf("%02x\r\n", uut->q_datb);
    if(uut->q_data != 0 || uut->q_datb != 0) {
        printf("Error in init!\r\n");
    }

    uut->i_clk      = 0;
    uut->i_reset    = 0;
    uut->eval();

    // Ready for use
    uut->i_clk      = 1;
    uut->i_en       = 1;
    uut->i_reset    = 0;
    uut->i_sela     = 0;
    uut->i_selb     = 0;
    uut->i_we       = 1;
    uut->i_seld     = 1;
    uut->i_datd     = 0xAB;
    uut->eval();

    uut->i_clk      = 0;
    uut->eval();

    uut->i_clk      = 1;
    uut->i_en       = 1;
    uut->i_reset    = 0;
    uut->i_sela     = 1;
    uut->i_selb     = 0;
    uut->i_we       = 0;
    uut->i_seld     = 0;
    uut->i_datd     = 0;
    uut->eval();
    printf("%02x\r\n", uut->q_data);
    if(uut->q_data != 0xAB) {
        printf("Error writing to register 1\r\n");
    }
    uut->i_clk      = 0;
    uut->eval();

    uut->i_clk      = 1;
    uut->i_en       = 1;
    uut->i_reset    = 0;
    uut->i_sela     = 0;
    uut->i_selb     = 0;
    uut->i_we       = 1;
    uut->i_seld     = 2;
    uut->i_datd     = 0xCD;
    uut->eval();

    uut->i_clk      = 0;
    uut->eval();

    uut->i_clk      = 1;
    uut->i_en       = 1;
    uut->i_reset    = 0;
    uut->i_sela     = 1;
    uut->i_selb     = 2;
    uut->i_we       = 0;
    uut->i_seld     = 0;
    uut->i_datd     = 0;
    uut->eval();
    printf("%02x\r\n", uut->q_data);
    printf("%02x\r\n", uut->q_datb);
    if(uut->q_data != 0xAB || uut->q_datb != 0xCD) {
        printf("Error writing to register 1 or 2\r\n");
    }
    uut->i_clk      = 0;
    uut->eval();

    delete uut;

    return 0;
}