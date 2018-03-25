//
// Created by BDL on 03/03/2018.
//

#include <assert.h>
#include "dbug.h"
#include "arch/template_impl.h"
#include "arch/prco_isa.h"
#include "gen.h"

#define ASM_OFFSET_BYTES 1

struct prco_op_struct asm_list[64] = {0};
int asm_list_it         = 0;

unsigned int g_asm_id   = 0;

int asm_tag_next        = 0;
int asm_tag_id          = 0;

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
        dt->cg_if = cg_if_template;

        eprintf(".text\r\n");
        eprintf(".globl _main\r\n");
        eprintf("_main:\r\n");
}

void asm_push(struct prco_op_struct op)
{
        asm_list[asm_list_it] = op;
        asm_list[asm_list_it].asm_offset = asm_list_it * ASM_OFFSET_BYTES;

        if(asm_tag_next) {
                asm_list[asm_list_it].asm_flags &= asm_tag_next;
                asm_list[asm_list_it].id = asm_tag_id;
                asm_tag_next = 0;
        }

        asm_list_it++;
}

#define asm_comment(s)                                                         \
        asm_list[(asm_list_it - 1) < 0 ? 0 : (asm_list_it-1)].comment = s;

#define asm_flags(i, f)                                                        \
        asm_list[(asm_list_it - i) < 0 ? 0 : (asm_list_it - 1)].flags f;

#define for_each_asm(it, asm_p)                                                \
        for (it = 0, asm_p = &asm_list[it];                                    \
                it < asm_list_it;                                              \
                it++, asm_p = &asm_list[it])

void asm_calc_labels(void)
{
        int     it, find;
        int     offset_check = 0x00;
        struct  prco_op_struct *op, *findop;

        dprintf(D_ALL, "asm_calc_labels:\r\n");

        for_each_asm(it, op) {
                assert(op->asm_offset == offset_check);
                offset_check += ASM_OFFSET_BYTES;

                // If we need to work out the return address
                // of a function
                if(op->asm_flags & ASM_CALL_NEXT) {
                        // Return addresses are placed into a register
                        // by a MOVI instsruction
                        assert(op->op == MOVI);
                        // 5 is the number of instructions to Create and push
                        //  the return pointer for the call
                        op->imm8 = op->asm_offset + (5 * ASM_OFFSET_BYTES);
                        // encode the new value in the opcode
                        op->opcode |= (op->imm8 & 0xff);
                        assert((op->opcode & 0xff) == op->imm8);

                        // Remove the flag as we are done with it
                        op->asm_flags &= ~ASM_CALL_NEXT;
                        continue;
                }

                // If we need to work out where to jump
                if(op->asm_flags & ASM_FUNC_CALL) {
                        struct ast_call *caller = (struct ast_call*)op->ast;

                        // Find offset of function entry
                        for_each_asm(find, findop) {
                                if(it == find) continue;

                                if ((findop->asm_flags & ASM_FUNC_START) && findop->ast) {
                                        struct ast_func *callee = (struct ast_func*)findop->ast;
                                        if(strcmp(caller->callee, callee->proto->name) == 0) {
                                                op->imm8 = findop->asm_offset;
                                                op->opcode |= op->imm8 & 0xFF;

                                                // Remove the flag
                                                op->asm_flags &= ~ASM_FUNC_CALL;
                                        }
                                }
                        }

                        continue;
                }
        }
}

int is_entry_func(struct ast_proto *p)
{
        return strcmp(p->name, "main") == 0;
}

void create_verilog_memh_file(void) {
        FILE    *fcoe;
        int     it;
        // asm instruction iterator
        struct prco_op_struct *op;

        fcoe = fopen("verilog_memh.txt", "w");
        if(!fcoe) {
                dprintf(D_ERR, "Unable to open uvm_coe.coe!\r\n");
                return;
        }

        // Write each instruction opcode on each line
        for_each_asm(it, op) {
                fprintf(fcoe, "%04x\n", op->opcode);
        }

        // Write top of stack address
        // /fprintf(fcoe, "@ff\n00ff");

        fclose(fcoe);
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

        /*
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
        */

        printf("\r\n\r\n");
}

void cg_postcode_template(void)
{
        int it;
        struct prco_op_struct *op;

        dprintf(D_INFO, "Postcode:\r\n");

        // Print each instruction in human readable format
        for_each_asm(it, op) {
                printf("0x%02X\t", op->asm_offset);
                assert_opcode(op, 1);
        }

        printf("\r\n\r\n");
        asm_calc_labels();

        // Print each instruction in human readable format
        for_each_asm(it, op) {
                printf("0x%02X\t", op->asm_offset);
                assert_opcode(op, 1);
        }

        // Debug
        // Inline verilog prco_lmem.v memory
        for_each_asm(it, op) {
                printf("r_lmem[%d] = 16'h%04x;\n", it, op->opcode);
        }

        // Write machine code to file
        create_verilog_memh_file();
}

void cg_push_prco(enum prco_reg rd)
{
        asm_push(opcode_add_ri(Sp, -1));
        asm_push(opcode_sw(rd, Sp, 0));
        asm_comment("PUSH");
}

void cg_pop_prco(enum prco_reg rd)
{
        asm_push(opcode_lw(rd, Sp, 0));
        asm_comment("POP");
        asm_push(opcode_add_ri(Sp, 1));
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
                case AST_NUM:   cg_number_template((struct ast_num*)e->expr);   break;
                case AST_BIN:   cg_bin_template((struct ast_bin*)e->expr);      break;
                case AST_CALL:  cg_call_template((struct ast_call*)e->expr);    break;
                case AST_IF:    cg_if_template((struct ast_if*)e->expr);        break;
                default: dprintf(D_ERR, "Unknown cg routine for %d\r\n",
                                 e->type);
                }
        }
}

inline void cg_sf_start(struct ast_func *f)
{
        struct prco_op_struct op;
        op = opcode_add_ri(Sp, -1);
        printf("SF START for %s\r\n", f->proto->name);
        op.ast = f;
        op.asm_flags |= ASM_FUNC_START;
        op.comment = "Function/sf entry";
        asm_push(op);
        asm_push(opcode_sw(Bp, Sp, 0));
        // Mov Sp -> Bp
        asm_push(opcode_mov_rr(Bp, Sp));
        asm_comment(f->proto->name);

        eprintf("push %%bp\r\n");
        eprintf("mov %%bp, %%sp\r\n");
}

void cg_sf_exit(void)
{
        // Mov Bp -> Sp
        asm_push(opcode_mov_rr(Sp, Bp));
        asm_comment("Function/sf exit");
        // Pop Bp
        cg_pop_prco(Bp);
}

void cg_call_template(struct ast_call *c)
{
        struct prco_op_struct op_next, op_call;
        op_next = opcode_mov_ri(Cx, 0x00);
        op_next.asm_flags = ASM_CALL_NEXT;
        op_next.comment = "Create return address";

        op_call = opcode_mov_ri(Cx, 0x00);
        op_call.asm_flags = ASM_FUNC_CALL;
        op_call.ast = c;
        op_call.comment = "call";

        // Set address of next function (current + 2)
        asm_push(op_next);
        cg_push_prco(Cx);

        // Set address of jmp
        asm_push(op_call);
        asm_push(opcode_jmp_r(Cx));
        asm_comment("JMP");
}

void cg_if_template(struct ast_if *v)
{
        struct prco_op_struct
                op_movi,
                op_dest,
                op_jmp;
        unsigned int jmp_id = ++g_asm_id;

        cg_expr_template(v->cond);

        asm_push(opcode_mov_ri(Cx, 0));
        asm_push(opcode_cmp_rr(Ax, Cx));

        // Create jmp location
        op_movi = opcode_mov_ri(Bx, 0x00);
        op_movi.asm_flags = ASM_JMP_JMP;
        op_movi.comment = "Jmp if false";
        op_movi.id = jmp_id;
        asm_push(op_movi);

        op_jmp = opcode_jmp_rc(Bx, JMP_JE);
        op_jmp.id = jmp_id;
        asm_push(op_jmp);

        // If true
        cg_expr_template(v->then);

        // After
        asm_tag_next = ASM_JMP_DEST;
        asm_tag_id   = jmp_id;
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
        cg_sf_start(f);

        // cg the function body
        cg_expr_template(f->body);

        // Create the stack exit
        // if required
        cg_sf_exit();

        // clean up
        cg_cur_function = NULL;

        printf("End function");

        if(is_entry_func(f->proto)) {
                asm_push(opcode_t1(HALT, 0, 0, 0));
                asm_comment("MAIN HALT\r\n------------------------------------");
        } else {
                cg_pop_prco(Cx);
                asm_push(opcode_jmp_r(Cx));
                asm_comment("FUNC RETURN to CALL\r\n---------------------------------------");
        }
        printf("\r\n");
}

void cg_bin_template(struct ast_bin *b)
{
        dprintf(D_GEN, "Starting cg for bin\r\n");

        cg_expr_template(b->lhs);

        if(b->rhs) {
                eprintf("PUSH %%ax\r\n");
                cg_push_prco(Ax);
                cg_expr_template(b->rhs);
        }

        switch(b->op) {
        case TOK_PLUS:
                eprintf("POP %%cx\r\n");
                cg_pop_prco(Cx);
                eprintf("ADD %%cx, %%ax\r\n");
                asm_push(opcode_add_rr(Ax, Cx));
                asm_comment("BIN ADD");
                break;

        case TOK_SUB: 
                eprintf("POP %%cx\r\n");
                cg_pop_prco(Cx);
                eprintf("SUB %%cx, %%ax");
                asm_push(opcode_sub_rr(Ax, Cx));
                asm_comment("BIN SUB");
                break;

        case TOK_STAR:
                eprintf("POP %%cx\r\n");
                break;

        default: assert("Unimplemented cg_bin_template b->op!" && 0); break;
        }
}

void cg_number_template(struct ast_num *n)
{
        eprintf("mov $%d, %%ax\r\n", n->val);
        asm_push(opcode_mov_ri(Ax, n->val));
        asm_comment("NUMBER");
}

void cg_var_template(struct ast_var *v) {}
void cg_local_decl_template(struct ast_lvar *v) {}