//
// Created by BDL on 03/03/2018.
//

#include <assert.h>
#include "dbug.h"
#include "arch/template_impl.h"

void cg_target_template_init(struct target_delegate *dt)
{
        dprintf(D_INFO, "cg: %s\r\n", __FUNCTION__);
        dt->cg_function = cg_function_template;
        dt->cg_bin = cg_bin_template;
        dt->cg_expr = cg_expr_template;
        dt->cg_local_decl = cg_local_decl_template;
        dt->cg_number = cg_number_template;
        dt->cg_var = cg_var_template;
}

inline void cg_sf_start(struct ast_func *f)
{
        dprintf(D_GEN, "PUSH EBP\r\n");
        dprintf(D_GEN, "MOV EPB, ESP\r\n");
        dprintf(D_GEN, "SUB ESP, 4\r\n");
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
        cg_sf_start(f);

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
                dprintf(D_GEN, "PUSH EAX\r\n");
                cg_expr_template(b->rhs);
        }

        switch(b->op) {
        case TOK_PLUS:
                dprintf(D_GEN, "POP ECX\r\n");
                dprintf(D_GEN, "ADD ECX, EAX\r\n"); break;
        default: assert("Unimplemented cg_bin_template b->op!" && 0);
        }
}

void cg_number_template(struct ast_num *n)
{

        dprintf(D_GEN, "Starting cg for number: %d\r\n", n->val);
        dprintf(D_GEN, "MOVI %d, EAX\r\n", n->val);
}

void cg_var_template(struct ast_var *v) {}
void cg_local_decl_template(struct ast_lvar *v) {}