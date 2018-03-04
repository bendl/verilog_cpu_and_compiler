/*
 * Copyright (c)
 */

#include <stdio.h>
#include "verilated.h"
#include "obj_dir/prco_core.h"

int main(int argc, char** argv) {
    prco_core *uut = new prco_core;

    Verilated::commandArgs(argc, argv);

    printf("Runing prco_core test...\r\n");

    uut->i_clk = 0;
    uut->i_reset = 1;
    uut->eval();

    uut->i_clk = 1;
    uut->eval();

    uut->i_reset = 0;
    uut->i_clk   = 0;
    uut->eval();

    uut->i_clk   = 1;
    uut->eval();

    uut->i_clk   = 0;
    uut->eval();

    uut->i_clk   = 1;
    uut->eval();

    uut->i_clk   = 0;
    uut->eval();

    uut->i_clk   = 1;
    uut->eval();

    uut->i_clk   = 0;
    uut->eval();

    uut->i_clk   = 1;
    uut->eval();

    uut->i_clk   = 0;
    uut->eval();

    printf("instr: %x\r\n", uut->prco_core__DOT__inst_decoder__DOT__i_instr);

    delete uut;

    return 0;
}