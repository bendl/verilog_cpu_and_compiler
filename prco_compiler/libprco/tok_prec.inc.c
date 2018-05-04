
#include "parser.h"
#include "dbug.h"

#ifndef LIBPRCO_TOK_PREC
#define LIBPRCO_TOK_PREC

/// Precedence parser table
/// \url http://wiki.tcl.tk/36951
int get_tok_prec(void)
{
        switch (lexer_token()) {
        default:
                dbprintf(D_INFO, "No token precedence for '%d'\r\n",
                        lexer_token());
                return -1;

        case TOK_BOOL_LE: return 80;
        case TOK_BOOL_L:  return 80;
        case TOK_BOOL_G:  return 80;
        case TOK_BOOL_EQ: return 80;
        case TOK_BOOL_NE: return 80;

        case TOK_BOOL_OR: return 140;

        case TOK_PLUS:    return 60;
        case TOK_SUB:     return 60;

        case TOK_STAR:    return 50;
        case TOK_DIV:     return 50;
        }
}

#endif
