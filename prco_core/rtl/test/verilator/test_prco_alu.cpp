/*
 * Copyright (c)
 */

#include <stdio.h>
#include "verilated.h"
#include "obj_dir/prco_alu.h"

#define TEST_SHOULD_PASS 1
#define TEST_SHOULD_FAIL 0

enum enum_instr {
        cmp =   0b01101,
        jmp =   0b01100,
        add =   0b01000,
        addi =  0b01001,
        ior =   0b10100,
        ixor =  0b10101,
        iand =  0b10110,
};

enum sr_flags {
        sr_z =  1,
        sr_s =  2,
        sr_c =  4,
        sr_o =  8,
};

enum enum_jmps {
        je =    1,
        jne,
        jg,
        jge,
        jl,
        jle,
        js,
        jns,
};

#define BINP8 "%c%c%c%c%c%c%c%c%c"
#define BIN8(byte)  \
        (byte & 0x0080 ? '1' : '0'), \
        (byte & 0x0040 ? '1' : '0'), \
        (byte & 0x0020 ? '1' : '0'), \
        (byte & 0x0010 ? '1' : '0'), \
        (byte & 0x0008 ? '1' : '0'), \
        (byte & 0x0004 ? '1' : '0'), \
        (byte & 0x0002 ? '1' : '0'), \
        (byte & 0x0001 ? '1' : '0')

#define array_size(a) (sizeof(a) / sizeof(a[0]))

struct test_item {
        char            *name;
        enum            enum_instr op;
        unsigned short  data;
        unsigned short  datb;
        signed char     imm8;
        signed char     simm5 : 5;
        unsigned short  result;
        unsigned short  expected;
        enum enum_jmps  jmps;
};
unsigned short last_cmp_result = 0;

int tests_passed = 0;
struct test_item tests[] = {
        { 
                .name = "ALU CMP eq", 
                .op = cmp, 
                .data = 0x000a,
                .datb = 0x000a,
                .imm8 = 0, 
                .simm5 = 0, 
                .result = 0,
                .expected = TEST_SHOULD_PASS, 
                .jmps = je
        },
        { 
                .name = "ALU CMP greater", 
                .op = cmp, 
                .data = 0x000b,
                .datb = 0x000a,
                .imm8 = 0, 
                .simm5 = 0, 
                .result = 0,
                .expected = TEST_SHOULD_PASS,  
                .jmps = jg
        },
        { 
                .name = "ALU CMP JGE", 
                .op = cmp, 
                .data = 0x000b,
                .datb = 0x000a,
                .imm8 = 0, 
                .simm5 = 0, 
                .result = 0,
                .expected = TEST_SHOULD_PASS,  
                .jmps = jge
        },
        { 
                .name = "ALU CMP JGE 2", 
                .op = cmp, 
                .data = 0x000b,
                .datb = 0x000b,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = TEST_SHOULD_PASS,  
                .jmps = jge
        },
        { 
                .name = "ALU CMP JLE", 
                .op = cmp, 
                .data = 0x000b,
                .datb = 0x000b,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = TEST_SHOULD_PASS,  
                .jmps = jle
        },
        { 
                .name = "ALU CMP JLE", 
                .op = cmp, 
                .data = 0x000c,
                .datb = 0x000b,
                .imm8 = 0, 
                .simm5 = 0, 
                .result = 0,
                .expected = TEST_SHOULD_FAIL,  
                .jmps = jle
        },
        { 
                .name = "ALU CMP less", 
                .op = cmp, 
                .data = 0x000a,
                .datb = 0x000b,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = TEST_SHOULD_PASS,  
                .jmps = jl
        },
        { 
                .name = "ALU CMP less (should fail)", 
                .op = cmp, 
                .data = 0x000b,
                .datb = 0x000a,
                .imm8 = 0, 
                .simm5 = 0, 
                .result = 0,
                .expected = TEST_SHOULD_FAIL,  
                .jmps = jl
        },
        { 
                .name = "ALU CMP JLE 2", 
                .op = cmp, 
                .data = 0x000a,
                .datb = 0x000b,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = TEST_SHOULD_PASS,  
                .jmps = jle
        },
        { 
                .name = "ALU ADD", 
                .op = add, 
                .data = 0x0001,
                .datb = 0x0002,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 0x0003
        },
        { 
                .name = "ALU ADDI", 
                .op = addi, 
                .data = 0x0000,
                .datb = 0x0001,
                .imm8 = 2, 
                .simm5 = 0, 
                .result =  0,
                .expected = 0x0003
        },
        
        // These test check whether a conditional branch should be taken based
        // on the previous CMP instruction.
        {
                .name = "ALU CMP JE", 
                .op = cmp, 
                .data = 0x000a,
                .datb = 0x000a,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 1,
                .jmps = je
        },
        {
                .name = "ALU JMP JE", 
                .op = jmp, 
                .data = 0x0000,
                .datb = 0x0000,
                .imm8 = je, 
                .simm5 = 0, 
                .result =  0,
                .expected = 1
        },
        {
                .name = "ALU JMP JG (fail)", 
                .op = jmp, 
                .data = 0x0000,
                .datb = 0x0000,
                .imm8 = jg, 
                .simm5 = 0, 
                .result =  0,
                .expected = TEST_SHOULD_FAIL
        },
        {
                .name = "ALU JMP JL (fail)", 
                .op = jmp, 
                .data = 0x0000,
                .datb = 0x0000,
                .imm8 = jl, 
                .simm5 = 0, 
                .result =  0,
                .expected = TEST_SHOULD_FAIL
        },
        {
                .name = "ALU JMP JLE", 
                .op = jmp, 
                .data = 0x0000,
                .datb = 0x0000,
                .imm8 = jle, 
                .simm5 = 0, 
                .result =  0,
                .expected = 1
        },

        // These test check whether a conditional branch should be taken based
        // on the previous CMP instruction.
        {
                .name = "ALU CMP JG", 
                .op = cmp, 
                .data = 0x000b,
                .datb = 0x000a,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 1,
                .jmps = jg
        },
        {
                .name = "ALU JMP JG", 
                .op = jmp, 
                .data = 0x0000,
                .datb = 0x0000,
                .imm8 = jg, 
                .simm5 = 0, 
                .result =  0,
                .expected = 1
        },
        {
                .name = "ALU JMP JG", 
                .op = jmp, 
                .data = 0x0000,
                .datb = 0x0000,
                .imm8 = jl, 
                .simm5 = 0, 
                .result =  0,
                .expected = TEST_SHOULD_FAIL
        },
        {
                .name = "ALU JMP NE", 
                .op = jmp, 
                .data = 0x0000,
                .datb = 0x0000,
                .imm8 = jne, 
                .simm5 = 0, 
                .result =  0,
                .expected = 1
        },

        // ALU bitwise operators
        {
                .name = "ALU OR", 
                .op = ior, 
                .data = 0x0000,
                .datb = 0x0000,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 0
        },
        {
                .name = "ALU OR 2", 
                .op = ior, 
                .data = 0x000a,
                .datb = 0x000a,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 0x000a
        },
        {
                .name = "ALU OR 3", 
                .op = ior, 
                .data = 0x0007,
                .datb = 0x0004,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 0x0007
        },
        // ALU AND
        {
                .name = "ALU AND 1", 
                .op = iand, 
                .data = 0x0003,
                .datb = 0x0002,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 0x0002
        },
        {
                .name = "ALU AND 2", 
                .op = iand, 
                .data = 0x0007,
                .datb = 0x0004,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 0x0004
        },
        {
                .name = "ALU AND 3", 
                .op = iand, 
                .data = 0xffff,
                .datb = 0xffff,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 0xffff
        },
        {
                .name = "ALU AND 4", 
                .op = iand, 
                .data = 0xff00,
                .datb = 0x0ff0,
                .imm8 = 0, 
                .simm5 = 0, 
                .result =  0,
                .expected = 0x0f00
        },
        
};

int check_flags_for_jmp(enum enum_jmps jmp, unsigned short sr) {
        switch(jmp) {
        case je:
                return (sr & sr_z == 1); 
                break;
        case jne:
                return (sr & sr_z == 0); 
                break;
        case jge:
                return (sr & sr_s) == (sr & sr_o);
                break;
        case jg:
                return ((sr & sr_z) == 0) & ((sr & sr_s) == (sr & sr_o));
                break;
        case jl:
                return ((sr & sr_s) != (sr & sr_o));
                break;
        case jle:
                return ((sr & sr_z) == 1) | ((sr & sr_s) != (sr & sr_o));
                break;
        default: 
                printf("Not implemented: %d\r\n", jmp);
                return 0;
                break;
        }
}

unsigned short run_test(prco_alu *uut, struct test_item *test) {
        unsigned short ret;
        uut->i_clk = 1;
        uut->i_ce    = 1;
        uut->i_op = test->op;
        uut->i_data = test->data;
        uut->i_datb  = (test->op == jmp) ? last_cmp_result : test->datb;
        uut->i_imm8 = test->imm8;
        uut->i_simm5 = test->simm5;
        uut->eval();

        test->result = (test->op == jmp) ? uut->q_should_branch : uut->q_result;

        if(test->op == cmp) {
                // Print out SR register bits
                printf("SR_Z: %d\r\n", test->result & sr_z);
                printf("SR_S: %d\r\n", test->result & sr_s);
                printf("SR_C: %d\r\n", test->result & sr_c);
                printf("SR_O: %d\r\n", test->result & sr_o);
                if (check_flags_for_jmp(test->jmps, test->result) == test->expected) {
                        printf("PASS: %d %d\r\n", test->result, test->expected);
                        tests_passed++;
                } else {
                        printf("FAIL!\r\n");
                }
                last_cmp_result = test->result;
        } else if(test->result != test->expected) {
                printf("FAIL: Got %d Expected %d\r\n", test->result, test->expected);
        } else {
                printf("PASS: %d %d\r\n", test->result, test->expected);
                tests_passed++;
        }

        uut->i_clk = 0;
        uut->eval();

        printf("\r\n");

        return ret;
}

void show_results(void) {
        printf("\r\n");
        printf("%d/%d tests passed.\r\n", tests_passed, array_size(tests));
        printf("\r\n");
}

int main(int argc, char** argv) {
        prco_alu *uut;
        int      i;

        uut = new prco_alu;

        Verilated::commandArgs(argc, argv);
        uut->i_clk = 0;
        uut->eval();

        for (i = 0; i < array_size(tests); i++) {
                printf("Running test: %s\r\n", tests[i].name);
                run_test(uut, &tests[i]);
        }

        show_results();

        return 0;
}
