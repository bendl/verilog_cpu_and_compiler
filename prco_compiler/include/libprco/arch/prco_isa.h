/*
 * Copyright (c)
 */

#ifndef PRCO_INSTR_H
#define PRCO_INSTR_H

#include "adt/ast.h"
#include "types.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif


#define BINP "%c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c"
#define BIN(byte)  \
  (byte & 0x8000 ? '1' : '0'), \
  (byte & 0x4000 ? '1' : '0'), \
  (byte & 0x2000 ? '1' : '0'), \
  (byte & 0x1000 ? '1' : '0'), \
  (byte & 0x0800 ? '1' : '0'), \
  (byte & 0x0400 ? '1' : '0'), \
  (byte & 0x0200 ? '1' : '0'), \
  (byte & 0x0100 ? '1' : '0'), \
  (byte & 0x0080 ? '1' : '0'), \
  (byte & 0x0040 ? '1' : '0'), \
  (byte & 0x0020 ? '1' : '0'), \
  (byte & 0x0010 ? '1' : '0'), \
  (byte & 0x0008 ? '1' : '0'), \
  (byte & 0x0004 ? '1' : '0'), \
  (byte & 0x0002 ? '1' : '0'), \
  (byte & 0x0001 ? '1' : '0')

  
#define BINP5 "%c%c%c%c%c"
#define BIN5(byte)  \
    (byte & 0x0010 ? '1' : '0'), \
    (byte & 0x0008 ? '1' : '0'), \
    (byte & 0x0004 ? '1' : '0'), \
    (byte & 0x0002 ? '1' : '0'), \
    (byte & 0x0001 ? '1' : '0')

typedef unsigned short regw_t;

struct prco_simm6 {
    signed char simm6 : 6;
};

#define PRCO_OP_BITS_OP   0b11111
#define PRCO_OP_BITS_REG  0b111
#define PRCO_OP_BITS_IMM8 0b11111111
#define PRCO_OP_BITS_PORT 0b11111111

struct prco_op_struct {
    regw_t              opcode;
    unsigned char       op;
    unsigned char       flags;  //< Depracated

    unsigned char       regD;
    unsigned char       regA;
    unsigned char       regB;

    unsigned char imm8  : 8;
    signed char   simm5 : 5;

    unsigned char       port;
    
    void *ast;
    unsigned char asm_offset;
    unsigned int asm_flags;
    char *comment;
    unsigned int id;
};

#define ASM_FUNC_EXIT   0x01
#define ASM_FUNC_START  0x02
#define ASM_IF_BRANCH   0x04
#define ASM_IF_ELSE     0x08
#define ASM_NOP_NOP     0x10
#define ASM_FUNC_CALL   0x20
#define ASM_CALL_NEXT   0x40

#define FOREACH_REG(REG) \
    REG(Ax) \
    REG(Bx) \
    REG(Cx) \
    REG(Dx) \
    REG(Ex) \
    REG(Sr) \
    REG(Bp) \
    REG(Sp) \
    REG(__prco_reg_MAX) \

#define FOREACH_PORT(PORT) \
        PORT(UART1) \
        PORT(GPIO1) \
        PORT(__prco_port_MAX)

#define FOREACH_OP(OP) \
    OP(NOP) \
    OP(LW) \
    OP(SW) \
    OP(MOV) \
    OP(MOVI) \
    OP(PUSH) \
    OP(POP) \
    OP(BEQ) \
    OP(ADD) \
    OP(ADDI) \
    OP(SUB) \
    OP(SUBI) \
    OP(JMP) \
    OP(CMP) \
    OP(NEG) \
    OP(CALL) \
    OP(RET) \
    OP(SPC) \
    OP(HALT) \
    OP(WRITE) \
    OP(READ) \
    OP(__prco_op_MAX) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STR(STR) #STR, 

enum prco_reg {
    FOREACH_REG(GENERATE_ENUM)
};

enum prco_op {
        FOREACH_OP(GENERATE_ENUM)
};

enum prco_port {
        FOREACH_PORT(GENERATE_ENUM)
};

STATIC_ASSERT(__prco_op_MAX <= PRCO_OP_BITS_OP+1, opcode_bit_length_exceeded); 
STATIC_ASSERT(__prco_reg_MAX <= PRCO_OP_BITS_REG+1, 3_bit_opcode_exceeded); 


static const char *REG_STR[] = { FOREACH_REG(GENERATE_STR) };
static const char *OP_STR[] = { FOREACH_OP(GENERATE_STR) };
static const char *PORT_STR[] = { FOREACH_PORT(GENERATE_STR) };


void assert_opcode(struct prco_op_struct *op, char print);
void print_opcode(struct prco_op_struct *prco_op);

struct prco_op_struct opcode_t1(enum prco_op iop, enum prco_reg rd, enum prco_reg ra,
                              signed char simm5);

struct prco_op_struct opcode_mov_rr(enum prco_reg regD, enum prco_reg regA);
struct prco_op_struct opcode_mov_ri(enum prco_reg regD, unsigned char imm8);
struct prco_op_struct opcode_mov_rri(enum prco_reg regA, enum prco_reg regD, signed char imm6);

struct prco_op_struct opcode_push_r(enum prco_reg regD);
struct prco_op_struct opcode_pop_r(enum prco_reg regD);

struct prco_op_struct opcode_add_rr(enum prco_reg regA, enum prco_reg regD);
struct prco_op_struct opcode_add_ri(enum prco_reg regD, signed char imm8);
struct prco_op_struct opcode_sub_rr(enum prco_reg regA, enum prco_reg regD);
struct prco_op_struct opcode_sub_ri(enum prco_reg regD, signed char imm8);

struct prco_op_struct opcode_jmp_r(enum prco_reg rd);
struct prco_op_struct opcode_cmp_rr(enum prco_reg rd, 
              enum prco_reg ra);
struct prco_op_struct opcode_call_i(unsigned char imm8);
struct prco_op_struct opcode_ret_i(unsigned char imm8);

struct prco_op_struct opcode_neg_r(enum prco_reg regD);

struct prco_op_struct opcode_lw(enum prco_reg rd, enum prco_reg ra, signed char imm5);
struct prco_op_struct opcode_sw(enum prco_reg rd, enum prco_reg ra, signed char imm5);

// TODO: Combine these instructions?
struct prco_op_struct opcode_read(enum prco_reg rd, enum prco_port port);
struct prco_op_struct opcode_write(enum prco_reg rd, enum prco_port port);

#ifdef __cplusplus
}
#endif

#endif // End include guard
