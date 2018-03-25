/*
MIT License

Copyright (c) 2017 Ben Lancaster

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

#ifndef LIBPRCO_AST_H
#define LIBPRCO_AST_H

#include "../parser.h"
#include "list.h"

/// Identifier for nodes
typedef enum ast_type {
        AST_NUM,
        AST_VAR,
        AST_BIN,
        AST_CALL,
        AST_LOCAL_VAR,
        AST_REF_VAR,
        AST_IF,
        AST_FOR,
        AST_BOOL,
        AST_ASSIGNMENT,
        AST_VAR_REF,

        AST_FUNC,
        AST_FUNC_EXIT,
        AST_RET,
} ast_type;


struct ast_item;

struct ast_num {
        int val;
};

struct ast_bin {
        enum token_type op;
        struct ast_item *lhs;
        struct ast_item *rhs;
};

struct ast_func {
        struct ast_proto *proto;
        struct ast_item  *body;
        struct ast_item  *exit;
        struct list_item *locals;
        struct ast_func  *next;

        int num_local_vars;
};

struct ast_proto {
        struct ast_func  *func;
        struct ast_proto *next;
        struct list_item *args;
        int              argc;
        char             *name;
};

struct ast_call {
        char                    *callee;
        struct list_item        *args;
        int                     argc;
        struct ast_func         *func;
        struct ast_proto        *proto;
};

struct ast_param {
        char *name;
        int  index;

        struct ast_param *next;
};

struct ast_var {
        char *name;
        int  dt;
        int  scope;
};

struct ast_lvar {
        struct ast_var *var;
        int            bp_offset;
};

struct ast_assign {
        struct ast_lvar *var;
        struct ast_item *val;
};

struct ast_if {
        struct ast_item *cond;
        struct ast_item *then;
        struct ast_item *els;

        int asm_id;
};

struct ast_for {
        struct ast_item *start;
        struct ast_item *cond;
        struct ast_item *step;
        struct ast_item *body;
};

struct ast_item {
        enum ast_type   type;
        struct ast_item *next;
        unsigned int    id;
        void            *expr;
};

struct ast_item_union {
        enum ast_type   type;
        struct ast_item *next;
        unsigned int    id;

        union {
                struct ast_num    num;
                struct ast_func   func;
                struct ast_proto  proto;
                struct ast_bin    bin;
                struct ast_if     aif;
                struct ast_for    afor;
                struct ast_assign assign;
                struct ast_lvar   lvar;
                struct ast_var    var;
                struct ast_param  param;
        };
};


// Instantiation functions
extern struct ast_item  *new_expr  (void *expr, enum ast_type type);
extern struct ast_num   *new_num   (int num_val);
extern struct ast_func  *new_func  (struct ast_proto *proto, struct ast_item *body);
extern struct ast_bin   *new_bin   (char op, struct ast_item *lhs, struct ast_item *rhs);
extern struct ast_proto *new_proto (char *name, struct list_item* args, int argc);
extern struct ast_var   *new_var   (char *name, int dt);
extern struct ast_lvar  *new_lvar  (struct ast_var *var);
extern struct ast_call  *new_call(char *callee, struct list_item *args, int argc);
extern struct ast_if    *new_if(struct ast_item *cond, struct ast_item *then, struct ast_item *els);


#endif