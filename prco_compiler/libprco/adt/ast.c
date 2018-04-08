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

#include "adt/ast.h"
#include "types.h"
#include <stdlib.h>
#include <assert.h>

void
ast_func_free(_inout_ struct ast_func *f)
{
        assert(f);
        assert(f->proto);

        ast_free(f->body);
}

void
ast_free(_inout_ struct ast_item *node)
{
        if(!node) {
                return;
        }

        if (node->next != NULL)
                ast_free(node->next);

        switch (node->type) {
        case AST_NUM:
                // Gets freed at bottom of ast_free
                break;

        case AST_IF: {
                struct ast_if *i = node->expr;
                ast_free(i->cond);
                ast_free(i->then);
                ast_free(i->els);
        } break;

        case AST_BIN: {
                struct ast_bin *b = node->expr;
                ast_free(b->lhs);
                ast_free(b->rhs);
        } break;

        case AST_CALL: {
                struct ast_call *c = node->expr;
                // TODO: Fix segfault

                //ast_free(c->args);
                //ast_free(c->proto); AST_FUNC will free AST_PROTO
                //ast_func_free(c->func);
        } break;

        case AST_FUNC: {
                struct ast_func *f = node->expr;
                ast_free(f->next); // This might cause a segfault
                ast_free(f->body);
                ast_free(f->exit);
                ast_free(f->proto);
        } break;

        } // End switch

        free(node);
}

struct ast_var *
ast_var_create(char *name, int dt)
{
        struct ast_var *ret = zalloc(ret);
        ret->name = name;
        ret->dt = dt;
        return ret;
}

struct ast_lvar *
ast_lvar_create(struct ast_var *var)
{
        struct ast_lvar *ret = zalloc(ret);
        ret->var = var;
        ret->bp_offset = 0;
        return ret;
}

struct ast_proto *
ast_proto_create(char *name,
                 struct list_item *args,
                 int argc)
{
        struct ast_proto *ret = zalloc(ret);
        ret->name = name;
        ret->args = args;
        ret->argc = argc;
        return ret;
}

struct ast_item *
ast_expr_create(void *expr, enum ast_type type)
{
        struct ast_item *ret = zalloc(ret);
        ret->expr = expr;
        ret->type = type;
        return ret;
}

struct ast_bin *
ast_bin_create(char op,
               struct ast_item *lhs,
               struct ast_item *rhs)
{
        struct ast_bin *ret = zalloc(ret);
        ret->op = op;
        ret->lhs = lhs;
        ret->rhs = rhs;
        return ret;
}

struct ast_func *
ast_func_create(struct ast_proto *proto,
                struct ast_item *body)
{
        struct ast_func *ret = zalloc(ret);
        ret->proto = proto;
        ret->body = body;
        ret->num_local_vars = 0;
        ret->exit = NULL;
        return ret;
}

struct ast_num *
ast_num_create(int num_val)
{
        struct ast_num *ret = zalloc(ret);
        ret->val = num_val;
        return ret;
}

struct ast_call *
ast_call_create(char *callee,
                struct list_item *args,
                int argc)
{
        struct ast_call *ret = zalloc(ret);
        ret->callee = callee;
        ret->args = args;
        ret->argc = argc;
        return ret;
}

struct ast_if *
ast_if_create(struct ast_item *cond,
              struct ast_item *then,
              struct ast_item *els)
{
        struct ast_if *ret = zalloc(ret);
        ret->cond = cond;
        ret->then = then;
        ret->els = els;
        return ret;
}

struct ast_for *
ast_for_create(struct ast_item *start,
               struct ast_item *cond,
               struct ast_item *step,
               struct ast_item *body)
{
        struct ast_for *ret = zalloc(ret);
        ret->start = start;
        ret->cond = cond;
        ret->step = step;
        ret->body = body;
        return ret;
}
