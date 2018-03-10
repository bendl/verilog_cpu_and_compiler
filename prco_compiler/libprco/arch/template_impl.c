//
// Created by BDL on 03/03/2018.
//

#include <assert.h>
#include "dbug.h"
#include "arch/template_impl.h"
#include "arch/prco_isa.h"

#define ASM_OFFSET_BYTES 1

struct prco_op_struct asm_list[64] = {0};
int asm_list_it   = 0;

#define asm_push(instr)                                                        \
    asm_list[asm_list_it] = instr;                                             \
    asm_list[asm_list_it].asm_offset = asm_list_it * ASM_OFFSET_BYTES;         \
    asm_list_it++;

#define asm_comment(s)                                                         \
    asm_list[(asm_list_it - 1) < 0 ? 0 : (asm_list_it-1)].comment = s;

#define asm_flags(i, f)                                                        \
    asm_list[(asm_list_it - i) < 0 ? 0 : (asm_list_it - 1)].flags f;

#define for_each_asm(it, asm_p)                                                \
        for (it = 0, asm_p = &asm_list[it];                                    \
             it < asm_list_it;                                                 \
             it++, asm_p = &asm_list[it])

void cg_target_template_init(struct target_delegate *dt)
{
        dprintf(D_INFO, "cg: %s\r\n", __FUNCTION__);
        dt->cg_function = cg_function_template;
        dt->cg_bin = cg_bin_template;
        dt->cg_expr = cg_expr_template;
        dt->cg_local_decl = cg_local_decl_template;
        dt->cg_number = cg_number_template;
        dt->cg_var = cg_var_template;
        dt->cg_postcode = cg_postcode_template;
        dt->cg_precode = cg_precode_template;

        eprintf(".text\r\n");
        eprintf(".globl _main\r\n");
        eprintf("_main:\r\n");
}

void cg_precode_template(void)
{
        int it;
        struct prco_op_struct op;

        dprintf(D_GEN, ".precode\r\n");
        for (it = NOP; it < __prco_op_MAX; it++) {
                //printf("%02x %s\r\n", it, OP_STR[it]);
                printf("`define PRCO_OP_%s\t5'b"BINP5"\n",
                OP_STR[it], BIN5(it));
        }

        for (it = UART1; it < __prco_port_MAX; it++) {
                //printf("%02x %s\r\n", it, OP_STR[it]);
                printf("`define PRCO_PORT_%s\t\t8'b"BINP5"\n",
                       PORT_STR[it], BIN5(it));
        }

        opcode_mov_ri(Ax, 0x10);
        opcode_mov_ri(Bx, 0x10);
        opcode_cmp_rr(Ax, Bx);
        opcode_mov_ri(Cx, 0x00);
        opcode_jmp_r(Cx);

        opcode_mov_ri(Ax, 0x42);
        opcode_write(Ax, UART1);
        opcode_mov_ri(Ax, 0x45);
        opcode_write(Ax, UART1);
        opcode_mov_ri(Ax, 0x4E);
        opcode_write(Ax, UART1);
        opcode_mov_ri(Ax, 0x32);
        opcode_write(Ax, UART1);
        opcode_mov_ri(Bx, 0x00);
        opcode_jmp_r(Bx);
}

void cg_postcode_template(void)
{

}

inline void cg_push_prco(enum prco_reg rd)
{
        asm_push(opcode_add_ri(Sp, -1));
        asm_push(opcode_sw(rd, Sp, 0));
        asm_comment("PUSH");
}

inline void cg_pop_prco(enum prco_reg rd)
{
        asm_push(opcode_lw(rd, Sp, 0));
        asm_comment("POP");
        asm_push(opcode_add_ri(Sp, 1));
}

inline void cg_sf_start(struct ast_func *f)
{
        eprintf("push %%bp\r\n");
        eprintf("mov %%bp, %%sp\r\n");
        eprintf("sub $4, %%sp\r\n");
}

void cg_debug_bin(struct ast_bin* b)
{
        cg_expr_template(b->lhs);
        if(b->rhs) {
                cg_expr_template(b->rhs);
        }
}

void cg_expr_template(struct ast_item *e)
{
        list_for_each(e) {
                switch(e->type) {
                case AST_NUM: cg_number_template((struct ast_num*)e->expr); break;
                case AST_BIN: cg_bin_template((struct ast_bin*)e->expr); break;
                default: dprintf(D_ERR, "Unknown cg routine for %d\r\n",
                                 e->type);
                }
        }
}

void cg_function_template(struct ast_func *f)
{
        dprintf(D_GEN, "Starting cg for function: %s\r\n",
                f->proto->name);

        assert(f);
        assert(f->proto);
        assert(f->body);

        cg_cur_function = f;

        // Create stack frame
        //cg_sf_start(f);

        // cg the function body
        cg_expr_template(f->body);

        // Create the stack exit
        // if required

        // clean up
        cg_cur_function = NULL;
}

void cg_bin_template(struct ast_bin *b)
{
        dprintf(D_GEN, "Starting cg for bin\r\n");

        cg_expr_template(b->lhs);

        if(b->rhs) {
                eprintf("PUSH %%ax\r\n");
                cg_expr_template(b->rhs);
        }

        switch(b->op) {
        case TOK_PLUS:
                eprintf("POP %%cx\r\n");
                eprintf("ADD %%cx, %%ax\r\n"); 
                break;

        case TOK_SUB: 
                eprintf("POP %%cx\r\n");
                eprintf("SUB %%cx, %%ax");
                break;

        default: assert("Unimplemented cg_bin_template b->op!" && 0); break;
        }
}

void cg_number_template(struct ast_num *n)
{
        eprintf("mov $%d, %%ax\r\n", n->val);
}

void cg_var_template(struct ast_var *v) {}
void cg_local_decl_template(struct ast_lvar *v) {}