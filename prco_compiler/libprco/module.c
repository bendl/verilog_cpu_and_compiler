//
// Created by BDL on 03/03/2018.
//

#include <stdlib.h>
#include "dbug.h"
#include "module.h"
#include <assert.h>

struct module *g_module;

struct module *new_module(void)
{
        struct module *ret;

        if(g_module) {
                dprintf(D_ERR, "ERR: Only 1 g_module can exist!\r\n");
                return R_ERROR;
        }

        ret = calloc(1, sizeof(*ret));
        g_module = ret;
        return ret;
}

struct module *get_g_module(void)
{
        assert(g_module);
        return g_module;
}
