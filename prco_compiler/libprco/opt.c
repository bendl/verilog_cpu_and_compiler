//
// Created by BDL on 03/03/2018.
//

#include <stdlib.h>
#include "dbug.h"
#include "opt.h"

struct ast_item *
opt_cf(struct ast_item *node)
{
        struct ast_item         *ret = node;

        // Tempory AST nodes
        struct ast_assign       *tmp_assign;
        struct ast_if           *tmp_if;
        struct ast_num          *tmp_num;
        struct ast_bin          *tmp_bin;
        struct ast_num          *tmp_bin_lhs;
        struct ast_num          *tmp_bin_rhs;
        // Replacement AST node
        struct ast_num          *new_num;

        switch(node->type) {

        // RHS of assignments might be foldable
        case AST_ASSIGNMENT:
                tmp_assign = node->expr;
                AST_SELF_CF(tmp_assign->val);
                break;

        // If statement conditions might be foldable
        case AST_IF:
                tmp_if = node->expr;
                AST_SELF_CF(tmp_if->cond);

                // If the condition has been folded to a constant,
                // we know the output of the if condition
                // So, remove the if condition and replace it with the
                // output
                if(tmp_if->cond->type == AST_NUM) {
                        tmp_num = tmp_if->cond->expr;
                        // If condition is true, replace with
                        // true body
                        if(tmp_num->val != 0) {
                                ret = tmp_if->then;
                                ret->next = node->next;
                        }
                        // Else its always false, replace it with
                        // the false body
                        else {
                                ret = tmp_if->els;
                                ret->next = node->next;
                        }
                }
                break;

        // Already reduced
        case AST_NUM:
                break;

        // Reducable if we can fold lhs and rhs
        case AST_BIN:
                tmp_bin = node->expr;
                AST_SELF_CF(tmp_bin->lhs);
                AST_SELF_CF(tmp_bin->rhs);

                if(tmp_bin->lhs->type == AST_NUM && tmp_bin->rhs->type == AST_NUM) {
                        tmp_bin_lhs = tmp_bin->lhs->expr;
                        tmp_bin_rhs = tmp_bin->rhs->expr;

                        // We can replace this, so create a new node to replace it
                        new_num = calloc(1, sizeof(*new_num));

                        // NOTE: These ops might differ
                        // from the processor architecture!
                        switch(tmp_bin->op) {
                        case TOK_PLUS:
                                new_num->val = tmp_bin_lhs->val + tmp_bin_rhs->val;
                                break;
                        case TOK_SUB:
                                new_num->val = tmp_bin_lhs->val - tmp_bin_rhs->val;
                                break;
                        case TOK_BOOL_L:
                                new_num->val = tmp_bin_lhs->val < tmp_bin_rhs->val;
                                break;
                        case TOK_BOOL_G:
                                new_num->val = tmp_bin_lhs->val > tmp_bin_rhs->val;
                                break;
                        default:
                                dbprintf(D_ERR, "OPT: CF: No CF OP for token: %c\r\n",
                                        tmp_bin->op);
                                break;
                        }

                        // Replace it
                        ret = alloc_expr(new_num, AST_NUM);
                        ret->next = node->next;
                        break;
                }
                break;
        }

        // Fold the next item
        if(node->next) {
                AST_SELF_CF(node->next);
        }

        return ret;
}