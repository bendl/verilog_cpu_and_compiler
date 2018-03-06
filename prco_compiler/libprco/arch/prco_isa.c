/*
 * Copyright (c)
 */

#include "arch/prco_isa.h"
#include <assert.h>

void assert_opcode(struct prco_op_struct *op, char print)
{
    assert(op);
    //printf("op: %s %d %s %s %s\r\n", OP_STR[prco_op->op], prco_op->flags,
      //  REG_STR[prco_op->regD], REG_STR[prco_op->regA], REG_STR[prco_op->regB]);
    switch(op->op) {
        case HALT:
            assert((op->opcode >> 11) == op->op);
            assert(((op->opcode >> 8) & PRCO_OP_BITS_REG) == op->regD);
            assert(((op->opcode >> 5) & PRCO_OP_BITS_REG) == op->regA);
            printf("%s\t\t\t%04x\t%s\r\n", 
                OP_STR[op->op],
                op->opcode, 
                op->comment);
            break;
            
        case LW:
        case SW:
            assert((op->opcode >> 11) == op->op);
            assert(((op->opcode >> 8) & PRCO_OP_BITS_REG) == op->regD);
            assert(((op->opcode >> 5) & PRCO_OP_BITS_REG) == op->regA);
            printf("%s\t%s,\t%+d(%s)\t%04x\t%s\r\n", 
                OP_STR[op->op],
                REG_STR[op->regD],
                op->simm5,
                REG_STR[op->regA],
                op->opcode,
                op->comment);
            break;
            
        case PUSH:
        case POP:
        case NEG:
        case JMP:
            assert((op->opcode >> 11) == op->op);
            assert(((op->opcode >> 8) & PRCO_OP_BITS_REG) == op->regD);
            printf("%s\t%s\t\t%04x\t%s\r\n", OP_STR[op->op],
                REG_STR[op->regD],
                op->opcode, 
                op->comment);
            break;

        case ADDI:
        case SUBI:
            assert((op->opcode >> 11) == op->op);
            assert(((op->opcode >> 8) & PRCO_OP_BITS_REG) == op->regD);
            assert(((op->opcode >> 0) & PRCO_OP_BITS_IMM8) == op->imm8);
            printf("%s\t$%+d,\t%s\t%04x\t%s\r\n", 
                OP_STR[op->op], (signed char)op->imm8,
                REG_STR[op->regD],
                op->opcode,
                op->comment);
            break;

        case CALL:
        case RET:
            assert((op->opcode >> 11) == op->op);
            assert(((op->opcode >> 0) & PRCO_OP_BITS_IMM8) == op->imm8);
            printf("%s\t$%02x\t\t%04x\t%s\r\n", OP_STR[op->op], op->imm8, 
                op->opcode,
                op->comment);
            break;

        case MOV:
        case ADD:
        case SUB:
        case CMP:
            assert((op->opcode >> 11) == op->op);
            assert(((op->opcode >> 8) & PRCO_OP_BITS_REG) == op->regD);
            assert(((op->opcode >> 5) & PRCO_OP_BITS_REG) == op->regA);
            printf("%s\t%s,\t%s\t%04x\t%s\r\n", 
                OP_STR[op->op],
                REG_STR[op->regA], 
                REG_STR[op->regD],
                op->opcode,
                op->comment);
            break;
        
        case MOVI:
            assert((op->opcode >> 11) == op->op);
            assert(((op->opcode >> 8) & PRCO_OP_BITS_REG) == op->regD);
            assert(((op->opcode >> 0) & PRCO_OP_BITS_IMM8) == op->imm8);
            printf("%s\t$%x,\t%s\t%04x\t%s\r\n", 
                OP_STR[op->op],
                op->imm8, 
                REG_STR[op->regD], 
                op->opcode,
                op->comment);
            break;
        default: printf("UNKNOWN\r\n"); break;
    }
    
}

void print_opcode(struct prco_op_struct *prco_op)
{
    switch(prco_op->op) {
        default: printf("\tUNKNOWN! %s\r\n", OP_STR[prco_op->op]); break;
    }
}

struct prco_op_struct opcode_t1(
    enum prco_op iop, 
    enum prco_reg rd, enum prco_reg ra, 
    signed char simm5)
{
    struct prco_op_struct op = { 0 };
    op.op = iop;
    op.regD = rd;
    op.regA = ra;
    op.simm5 = (simm5 & 0b11111);
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.regA << 5;
    op.opcode |= (op.simm5 & 0b11111) << 0;

    return op;
}

struct prco_op_struct opcode_mov_ri(enum prco_reg regD, unsigned char imm8)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = MOVI;
    op.regD = regD;
    op.imm8 = imm8;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.imm8 << 0;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_mov_rr(enum prco_reg regD, enum prco_reg regA)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = MOV;
    op.regD = regD;
    op.regA = regA;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.regA << 5;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_push_r(enum prco_reg regD)
{
    struct prco_op_struct op = { 0 };
    assert(0 && "Depracted! Use cg_push");
    op.flags = 0;
    op.op = PUSH;
    op.regD = regD;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_pop_r(enum prco_reg regD)
{
    struct prco_op_struct op = { 0 };
    assert(0 && "Depracted! Use cg_pop");
    op.flags = 0;
    op.op = POP;
    op.regD = regD;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_add_rr(enum prco_reg regD, enum prco_reg regA)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = ADD;
    op.regD = regD;
    op.regA = regA;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.regA << 5;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_add_ri(enum prco_reg regD, signed char imm8)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = ADDI;
    op.regD = regD;
    op.imm8 = imm8;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.imm8 << 0;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_sub_ri(enum prco_reg regD, signed char imm8)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = SUBI;
    op.regD = regD;
    op.imm8 = imm8;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.imm8 << 0;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_sub_rr(enum prco_reg regD, enum prco_reg regA)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = SUB;
    op.regD = regD;
    op.regA = regA;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.regA << 5;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_jmp_r(enum prco_reg rd)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = JMP;
    op.regD = rd;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct 
opcode_cmp_rr(enum prco_reg rd, 
              enum prco_reg ra, 
              enum prco_reg rb)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = CMP;
    op.regD = rd;
    op.regA = ra;
    op.regB = rb;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.regA << 5;
    op.opcode |= op.regB << 2;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_call_i(unsigned char imm8)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = CALL;
    op.opcode |= op.op << 11;
    op.opcode |= op.imm8 << 0;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_ret_i(unsigned char imm8)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = RET;
    op.opcode |= op.op << 11;
    op.opcode |= op.imm8 << 0;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_neg_r(enum prco_reg regD)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = NEG;
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_lw(enum prco_reg rd, enum prco_reg ra, signed char imm5)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = LW;
    op.regD = rd;
    op.regA = ra;
    op.simm5 = (imm5 & 0b11111);
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.regA << 5;
    op.opcode |= (op.simm5 & 0b11111) << 0;

    assert_opcode(&op, 0);
    return op;
}

struct prco_op_struct opcode_sw(enum prco_reg rd, enum prco_reg ra, signed char imm5)
{
    struct prco_op_struct op = { 0 };
    op.flags = 0;
    op.op = SW;
    op.regD = rd;
    op.regA = ra;
    op.simm5 = (imm5 & 0b11111);
    op.opcode |= op.op << 11;
    op.opcode |= op.regD << 8;
    op.opcode |= op.regA << 5;
    op.opcode |= (op.simm5 & 0b11111) << 0;

    assert_opcode(&op, 0);
    return op;
}
