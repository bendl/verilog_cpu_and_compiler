//
// Created by BDL on 03/03/2018.
//

#include "gen.h"
#include "module.h"
#include <stdarg.h>

void cg_dump(struct module *m,
             enum target_arch arch)
{
        // Initialise the target code generator
        init_target(arch);


        struct ast_proto *p = m->prototypes;
        struct ast_func  *f = m->functions;

        list_for_each(f) {
                cg_target_delegate.cg_function(f);
        }
}

void eprintf(char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        vfprintf(g_file_out, fmt, args);
        va_end(args);

        if (module_dump) {
                va_start(args, fmt);
                vprintf(fmt, args);
                va_end(args);
        }
}