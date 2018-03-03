//
// Created by BDL on 19/02/2018.
//

#include "arch/target.h"
#include "arch/template_impl.h"

struct target_delegate  cg_target_delegate;
struct ast_func         *cg_cur_function;

void init_target(enum target_arch arch)
{
        // Zero out the target delegate
        //cg_target_delegate = {0};

        switch(arch) {
        default:
        case target_generic:
        case target_template:
                cg_target_template_init(&cg_target_delegate);
                break;
        }
}