/*
MIT License

Copyright (c) 2018 Ben Lancaster

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifdef _MSC_VER
#include "getopt.h"
#else
#include <getopt.h>
#endif


#include <libprco/dbug.h>
#include <libprco/parser.h>
#include <libprco/module.h>
#include <libprco/arch/prco_isa.h>
#include <libprco/arch/prco_impl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct prco_emu_core {
        unsigned short pc;
        unsigned short r_regs[8];
        unsigned char r_sr;

        unsigned short lmem[0xff];
};

struct prco_emu_core core = {0};
struct prco_op_struct current_op = {0};

void dbug_print_regs(void)
{
        int i;
        printf("\t\t\t\t\t\t\t\t");

        for(i = 0; i < 8; i++) {
                printf("%02x ", core.r_regs[i]);
        }
        printf("\r\n");
}

void dbug_print_mem(void)
{
        int i;
        int width = 8;
        int i2;

        for (i = 0; i <= width; i++) {
                printf("%02x\t", i);
        }

        printf("\r\n"
               "============================="
               "============================="
               "============================="
                "\r\n");

        for (i = 0; i < 0xff; i++) {
                dprintf(D_INFO, "%02x\t", core.lmem[i]);
                if(i2 == width) {
                        dprintf(D_INFO, "\r\n");
                        i2 = 0;
                } else {
                        i2++;
                }
        }
        dprintf(D_INFO, "\r\n");
}

void emu_load_mem(char *fpath)
{
        int i = 0;
        FILE *f;
        char buf[32];
        int buf_i = 0;
        char c = 0;

        f = fopen(fpath, "r");
        if(!f) return;

        while((c = fgetc(f)) != EOF) {
                if(isalnum(c)) {
                        buf[buf_i++] = c;
                }

                if (c == '\n') {
                        buf[buf_i] = 0;
                        int num = strtol(buf, NULL, 16);
                        //dprintf(D_INFO, "Read Word: '%s' %x\r\n", buf, num);
                        buf_i = 0;
                        core.lmem[i++] = num;
                }
        }

}

void emu_exec(struct prco_op_struct *op)
{
        switch(op->op) {
        case NOP:
                break;
        case ADDI:
                core.r_regs[op->regD] += (signed char)op->imm8;
                dbug_print_regs();
                break;
        case SUBI:
                core.r_regs[op->regD] -= (signed char)op->imm8;
                dbug_print_regs();
                break;
        case MOV:
                core.r_regs[op->regD] = core.r_regs[op->regA];
                dbug_print_regs();
                break;
        case MOVI:
                core.r_regs[op->regD] = op->imm8;
                dbug_print_regs();
                break;
        case SW:
                core.lmem[core.r_regs[op->regA + op->simm5]] = core.r_regs[op->regD];
                break;
        case LW:
                core.r_regs[op->regD] = core.lmem[core.r_regs[op->regA + op->simm5]];
                dbug_print_regs();
                break;
        }
}

struct prco_op_struct emu_decode(unsigned short mc)
{
        struct prco_op_struct dec = {0};
        dec.op = mc >> 11;
        dec.regD = (mc >> 8) & 0b111;
        dec.regA = (mc >> 5) & 0b111;
        dec.imm8 = (mc >> 0) & 0xff;
        dec.simm5 = (mc & 0b11111);
        dec.opcode = mc;

        assert_opcode(&dec, 0);

        return dec;
}

int main(int argc, char **argv)
{
        g_dbug_level = 0xff;

        emu_load_mem("verilog_memh.txt");
        dbug_print_mem();
        core.r_regs[7] = 0xff;
        dbug_print_regs();

        while(1) {
                if(core.pc > 0x30) break;

                current_op = emu_decode(core.lmem[core.pc]);
                emu_exec(&current_op);

                core.pc++;
        }


        dbug_print_mem();
        dbug_print_regs();
        return 0;
}