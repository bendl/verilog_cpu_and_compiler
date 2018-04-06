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
#include <ctype.h>

#define LIBPRCO_EMU_VERSION 2.60

char uart_tx_buf[0xff];
char uart_tx_buf_i = 0;

struct prco_emu_core {
        unsigned short pc;
        unsigned short r_regs[8];
        unsigned short r_sr;

        unsigned short lmem[0xff];

        struct prco_op_struct current_op;

        int should_branch;

        // stats
        int exec_count;
};

struct prco_emu_core core = {0};

#define PRINT_SPACE(d) \
        dprintf((d), "\t\t\t\t\t\t\t\t\t");

struct prco_op_struct   emu_decode(unsigned short mc);
void                    emu_exec(struct prco_op_struct *op);

void print_regs(void)
{
        int i;

        PRINT_SPACE(D_EMU);
        for(i = 0; i < 8; i++) {
                dprintf(D_EMU, "%02x ", core.r_regs[i]);
        }
        dprintf(D_EMU, "\r\n");
}

void print_mem(void)
{
        int i = 0;
        int width = 8;
        int i2 = 0;

        dprintf(D_EMU, "\r\n\r\n");

        for (i = 0; i < width; i++) {
                dprintf(D_EMU, "%02x\t", i);
        }

        dprintf(D_EMU, "\r\n"
               "============================="
               "============================="
               "============================="
                "\r\n");

        for (i = 0; i <= 0xff; i++) {
                if(i == core.r_regs[7])
                        dprintf(D_EMU, "SP");
                if(i == core.r_regs[6])
                        dprintf(D_EMU, "BP");

                dprintf(D_EMU, "%02x\t", core.lmem[i]);
                if(i2 == width-1) {
                        dprintf(D_EMU, "\r\n");
                        i2 = 0;
                } else {
                        i2++;
                }
        }
        dprintf(D_EMU, "\r\n");
}

void print_uart(void)
{
        dprintf(D_EMU, "UART tx buf:\r\n");
        dprintf(D_EMU, "%s\r\n", uart_tx_buf);
}

unsigned short
alu_cmp(struct prco_emu_core *core,
        unsigned short a, unsigned short b)
{
        unsigned long tmp = 0;
        unsigned short ret = 0;

        tmp = a - b;
        PRINT_SPACE(D_EMU2);
        dprintf(D_EMU2, "ALU_TMP(%d bits): %x\r\n",
                sizeof(tmp)*8, tmp);

        // Zero flag
        if(tmp == 0) ret |= 1;

        // Signed flag
        ret |= ((tmp & 0x8000) >> 14);

        // Overflow flag
        switch((tmp & 0xC000) >> 13) {
        case 0b01:
        case 0b10:
                ret |= (1 >> 2);
                break;
        default:
                ret |= (0 >> 2);
        }

        PRINT_SPACE(D_EMU2);
        dprintf(D_EMU2, "ALU_CMP: %02x %02x = %02x\r\n",
                a, b, ret);

        return ret;
}

unsigned short
alu_should_jmp(unsigned char imm8)
{
        unsigned short ret = 0;

        switch(imm8) {
        case 0:
                ret = 1;
                break;
        case JMP_JE:
                ret = (core.r_sr & 0b1) == 1;
                break;
        case JMP_JNE:
                ret = (core.r_sr & 0b1) == 0;
                break;
        case JMP_JS:
                ret = ((core.r_sr & 0b10) >> 1) ==  0;
                break;
        case JMP_JG:
                ret = (
                        ((core.r_sr & 0b1) == 0) &
                        (
                                ((core.r_sr & 0b10) >> 1) == ((core.r_sr & 0b100) >> 2)
                        )
                );
                break;
        case JMP_JL:
                ret = (
                        ((core.r_sr & 0b10) >> 1) != ((core.r_sr & 0b100) >> 2)
                );
                break;
        default:
                ret = 0;
        }


        return ret;
}

unsigned short
alu_should_set(unsigned char imm8)
{
        dprintf(D_EMU2, "SR REG: %02x\r\n", core.r_sr);
        switch(imm8) {
        case JMP_JE:
                return (core.r_sr & 0b1) == 1;
        case JMP_JL:
                return ((core.r_sr & 0b10) >> 1) != ((core.r_sr & 0b100) >> 2);
        case JMP_JG:
                return ((core.r_sr & 0b1) == 0) &
                       (
                               ((core.r_sr & 0b10) >> 1) == ((core.r_sr & 0b100) >> 2)
                       );
        default:
                return 0;
        }
}

void emu_load_mem(char *fpath)
{
        int i = 0;
        FILE *f;
        char buf[32];
        int buf_i = 0;
        char c = 0;
        unsigned short mc;
        struct prco_op_struct op;

        f = fopen(fpath, "r");
        if(!f) return;

        while((c = fgetc(f)) != EOF) {
                if(isalnum(c)) {
                        buf[buf_i++] = c;
                }

                if (c == '\n') {
                        buf[buf_i] = 0;
                        mc = strtol(buf, NULL, 16);
                        core.lmem[i++] = mc;

                        emu_decode(mc);

                        buf_i = 0;
                }
        }

        fclose(f);
}

void emu_exec(struct prco_op_struct *op)
{
        switch(op->op) {
        default:
                dprintf(D_EMU,
                        "UKNOWN OP: %s %x\r\n", OP_STR[op->op],
                        op->op);
                break;
        case NOP:
                break;
        case ADD:
                core.r_regs[op->regD] += core.r_regs[op->regA];
                print_regs();
                break;
        case ADDI:
                core.r_regs[op->regD] += (signed char)op->imm8;
                print_regs();
                break;
        case SUB:
                core.r_regs[op->regD] = core.r_regs[op->regD] - core.r_regs[op->regA];
                print_regs();
                break;
        case SUBI:
                core.r_regs[op->regD] -= (signed char)op->imm8;
                print_regs();
                break;
        case MOV:
                core.r_regs[op->regD] = core.r_regs[op->regA];
                print_regs();

                // We can guess this instruction is creating
                // a new stack frame, so print new Bp and Sp locations
                if(op->regD == Bp && op->regA == Sp)
                        print_mem();

                break;
        case MOVI:
                core.r_regs[op->regD] = op->imm8;
                print_regs();
                break;
        case SW:
                core.lmem[core.r_regs[op->regA] + op->simm5] = core.r_regs[op->regD];
                PRINT_SPACE(D_EMU2);
                dprintf(D_EMU2, "SW $%02x, mem[%02x]\r\n",
                        core.r_regs[op->regD],
                        core.r_regs[op->regA + op->simm5]);
                print_mem();
                break;
        case LW:
                core.r_regs[op->regD] = core.lmem[core.r_regs[op->regA] + op->simm5];
                PRINT_SPACE(D_EMU2);
                dprintf(D_EMU2, "LW mem[%02x], $%02x\r\n",
                        core.r_regs[op->regA + op->simm5],
                        core.r_regs[op->regD]);
                print_regs();
                break;

        case CMP:
                core.r_sr = alu_cmp(
                        &core,
                        core.r_regs[op->regD],
                        core.r_regs[op->regA]);
                dprintf(D_EMU2, "SR REG: %02x\r\n", core.r_sr);
                break;
        case JMP:
                core.should_branch = alu_should_jmp(op->imm8);
                break;

        case SET:
                core.r_regs[op->regD] = alu_should_set(op->imm8);
                print_regs();
                break;

        case WRITE:
                PRINT_SPACE(D_EMU2);
                dprintf(D_EMU2, "PORT %d\r\n", op->port);

                switch(op->port) {
                case UART1:
                        PRINT_SPACE(D_EMU2);
                        dprintf(D_EMU,
                                "UART <- '%c' 0x%x\r\n",
                                core.r_regs[op->regD],
                                core.r_regs[op->regD]);
                        uart_tx_buf[uart_tx_buf_i++] = core.r_regs[op->regD];
                        break;
                }

                break;
        }

        core.exec_count++;
}

struct prco_op_struct emu_decode(unsigned short mc)
{
        struct prco_op_struct dec = {0};
        dec.op = mc >> 11;
        dec.regD = (mc >> 8) & 0b111;
        dec.regA = (mc >> 5) & 0b111;
        dec.imm8 = (mc >> 0) & 0xff;
        dec.simm5 = (mc & 0b11111);
        dec.port = (mc & 0xff);
        dec.opcode = mc;

        assert_opcode(&dec, 0);

        return dec;
}

int emu_init(struct prco_emu_core *core)
{
        memset(core, 0, sizeof(*core));
        core->r_regs[7] = 0xff;

        memset(uart_tx_buf, 0, 0xff);

        // Load memory with instruction data
        emu_load_mem("verilog_memh.txt");
}

int emu_run(struct prco_emu_core *core)
{
        while(1) {
                dprintf(D_EMU, "0x%02x ", core->pc);

                // Decode the instruction
                core->current_op = emu_decode(core->lmem[core->pc]);

                if(core->current_op.op == HALT
                   || core->exec_count > 20000)
                        break;

                // Execute the instruction
                emu_exec(&core->current_op);

                // Update pc
                if(core->should_branch) {
                        PRINT_SPACE(D_EMU2);
                        core->pc = core->r_regs[core->current_op.regD];
                        dprintf(D_EMU2, "Branching to %02x\r\n", core->pc);
                        core->should_branch = 0;
                } else {
                        core->pc++;
                }
        }
}

int main(int argc, char **argv)
{
        // Turn on all debug prints
        g_dbug_level = 0xff;

        dprintf(D_INFO, "LIBPRCO Emulator. Version %f\r\n\r\n",
                LIBPRCO_EMU_VERSION);

        // Initialise the core
        emu_init(&core);

        // Print default registers and memory
        print_mem();
        print_regs();

        printf("alu: %x\r\n", alu_cmp(&core, 5, 5));

        // Run the emulator
        emu_run(&core);

        // After, print registers and memory
        dprintf(D_EMU, "\r\nCore HALTED after %d executions.\r\n",
                core.exec_count);
        print_mem();
        print_regs();

        print_uart();

        // Return Ax register for testing
        return core.r_regs[0];
}