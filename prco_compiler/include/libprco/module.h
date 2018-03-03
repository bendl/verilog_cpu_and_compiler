//
// Created by BDL on 03/03/2018.
//

#ifndef LIBPRCO_TOP_MODULE_H
#define LIBPRCO_TOP_MODULE_H

#include "parser.h"

struct module {
        struct ast_proto *prototypes;
        struct ast_func  *functions;
        struct ast_func  *entry;
        struct ast_item  *top_levels;
        struct ast_item  *top_levels_last;
};

extern struct module *g_module;

extern struct module *get_g_module(void);

extern struct module *new_module(void);

#endif //LIBPRCO_TOP_MODULE_H
