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

#ifndef LIBPRCO_TARGET_H
#define LIBPRCO_TARGET_H

#include "../adt/ast.h"

/// Supported backend targets
/// \brief
typedef enum target_arch {
    target_generic,  ///< A generic assembly target
    target_template, ///< A template assembly target, for starting with
    target_8086,     ///< 8086 intel syntax target
    target_x86,      ///< x86 at&t syntax target
    target_prco100,   ///< https://github.com/bendl/prco target
} target_archs;

/// Target datatype sizes
/// \brief
/// \see get_dt_size
typedef enum target_datatype {
    dtINT,  ///< Int
    dtPTR,  ///< Pointer
    dtCHAR, ///< 1 byte
} target_datatype;


typedef void (*cg_precode_d)    (void);
typedef void (*cg_postcode_d)   (void);
typedef void (*cg_expr_d)       (struct ast_item *t);
typedef void (*cg_function_d)   (struct ast_func *f);   ///< Cg function
typedef void (*cg_bin_d)        (struct ast_bin *b);    ///< Cg binary expression
typedef void (*cg_number_d)     (struct ast_num *n);    ///< Cg number expression
typedef void (*cg_var_d)        (struct ast_var *v);    ///< Cg variable/ident reference
typedef void (*cg_call_d)       (struct ast_call *c);   ///< Cg function call
typedef void (*cg_local_decl_d) (struct ast_lvar *v);   ///< Cg variable declaration
typedef void (*cg_if_d)         (struct ast_if *v);     ///< Cg if expression

/*
typedef void (*cg_if_d)(ast_if_t *i);           ///< Cg if expression
typedef void (*cg_for_d)(ast_for_t *f);         ///< Cg for loop expression
typedef void (*cg_assignment_d)(ast_assign_t *a); ///< Cg variable assignment
typedef void (*cg_var_ref_d)(ast_lvar_t *v); ///< Cg local variable reference
typedef void (*cg_dir_extern_d)(ast_proto_t *p);  ///< Cg extern declaration
typedef void (*cg_func_ret)(ast_ret_t *r);        ///< Cg extern declaration
typedef int (*get_dt_size_d)(target_datatype dt); ///< Target datatype
*/

/// Structure containing function pointers to target specific codegen functions
/// \brief
struct target_delegate {
        cg_precode_d    cg_precode;
        cg_postcode_d   cg_postcode;
        cg_expr_d       cg_expr;
        cg_function_d   cg_function;
        cg_bin_d        cg_bin;
        cg_number_d     cg_number;
        cg_var_d        cg_var;
        cg_call_d       cg_call;
        cg_local_decl_d cg_local_decl;

        cg_if_d         cg_if;
        /*
        cg_for_d        cg_for;
        cg_assignment_d cg_assignment;
        cg_var_ref_d    cg_var_ref;
        cg_dir_extern_d cg_dir_extern;
        cg_func_ret     cg_func_ret;
        get_dt_size_d   get_dt_size;
        */
};

extern struct target_delegate   cg_target_delegate;
extern enum target_arch         cg_target_arch;
extern struct ast_func          *cg_cur_func;

extern void init_target(enum target_arch arch);

#endif