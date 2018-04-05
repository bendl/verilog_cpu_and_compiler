//
// Created by BDL on 19/02/2018.
//

#ifndef LIBPRCO_TYPES_H
#define LIBPRCO_TYPES_H

#define true 1
#define TRUE true

#define false 0
#define FALSE false

#define R_OK 0
#define R_ERROR 1

// Function signiture helpers
#define _in_
#define _inout_
#define _out_

typedef char *pstr;

#define DBUG_INFO "INFO: "
#define DBUG_WARN "WARN: "
#define CG_DBUG "\t#DEBUG "

// Terminal colours
#define BLU "\e[0;34m"
#define RED "\e[0;31m"
#define YEL "\e[0;33m"
#define RST "\e[0;0m"

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]

#define zalloc(t) \
        calloc(1, sizeof(*t))

#endif //LIBPRCO_TYPES_H
