//
// Created by BDL on 03/03/2018.
//

#include <stdlib.h>
#include <assert.h>

#include "dbug.h"
#include "module.h"
#include "types.h"
#include "gen.h"

struct module *g_module;
FILE *         g_file_out;

struct module *new_module(void)
{
        struct module *ret;

        if(g_module) {
                dprintf(D_ERR, "ERR: Only 1 g_module can exist!\r\n");
                return NULL;
        }

        ret = zalloc(ret);
        g_module = ret;
        return ret;
}

struct module *get_g_module(void)
{
        assert(g_module);
        return g_module;
}

void module_dump(struct module *m)
{
        cg_dump(m, target_template);
}

void
module_free(struct module *m)
{
        struct ast_func *func_it = m->functions;

        list_for_each(func_it) {
                ast_func_free(func_it);
        }
}

