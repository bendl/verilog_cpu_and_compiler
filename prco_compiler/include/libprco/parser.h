//
// Created by BDL on 19/02/2018.
//

#ifndef LIBPRCO_PARSER_H
#define LIBPRCO_PARSER_H


#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#include <stdio.h>

#define COMPILER_INCLUDE_ENV_VAR "LIBPRCO_INCLUDE_PATH"
#define PARSER_ERROR_STACK 4
#define PARSER_ERROR_FOPEN 5

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STR(STR) #STR,

// Lexer definitions
#define FOREACH_TOK(TOK) \
    TOK(TOK_EOF) \
    TOK(TOK_DEF) \
    TOK(TOK_EXT) \
    TOK(TOK_ID) \
    TOK(TOK_NUM) \
    TOK(TOK_PLUS) \
    TOK(TOK_SUB) \
    TOK(TOK_STAR) \
    TOK(TOK_DIV) \
    TOK(TOK_LESS) \
    TOK(TOK_LBRACE) \
    TOK(TOK_RBRACE) \
    TOK(TOK_LCBRACE) \
    TOK(TOK_RCBRACE) \
    TOK(TOK_COMMA) \
    TOK(TOK_ASSIGNMENT) \
    TOK(TOK_IF) \
    TOK(TOK_ELSE) \
    TOK(TOK_BOOL_EQ) \
    TOK(TOK_BOOL_NE) \
    TOK(TOK_BOOL_N) \
    TOK(TOK_BOOL_L) \
    TOK(TOK_BOOL_LE) \
    TOK(TOK_BOOL_G) \
    TOK(TOK_BOOL_GE) \
    TOK(TOK_BOOL_OR) \
    TOK(TOK_BIT_OR) \
    TOK(TOK_FOR) \
    TOK(TOK_DQUOTE) \
    TOK(TOK_QUOTE) \
    TOK(TOK_RET) \
    TOK(TOK_CALL) \
    TOK(TOK_VARIABLE) \
    TOK(TOK_COMMENT) \
    TOK(TOK_VARIADIC) \
    TOK(TOK_ERROR) \
    TOK(TOK_PREP_DIRECTIVE) \
    TOK(TOK_CC_STDCALL) \
    TOK(TOK_CC_CDECL) \
    TOK(TOK_CC_FASTCALL) \
    TOK(TOK_PORT_PORT1) \
    TOK(TOK_PORT_UART1)

/// lexer token types
typedef enum token_type {
    FOREACH_TOK(GENERATE_ENUM)
} token_type;

static const char *TOK_STR[] = { FOREACH_TOK(GENERATE_STR) };


/// Parser context
///
/// Contains FILE reference, file paths, and current parser state variables
struct text_parser {
    char *lpstr_input_fp;   //< Full path to the file
    char *lpstr_input_dir;  //< Full path to the directory of the file
    FILE *file_input;       //< FILE handle
    int   i_file_line;      //< Lexer line number
    int   i_file_col;       //< Lexer line column
    int   parse_result;   //< Integer parse result, 1 = ok
};

/// Reserved word structure
///
struct resv_word {
    char *     lpstr_name;
    int        dt_size;
    token_type tok;
};

/// Lexer current token
extern int g_cur_token;

/// Lexer current number value
extern int g_num_val;

/// Lexer current ident string
extern char *ident_str;

/// Lexer input file
extern FILE *g_file_input;
extern char *g_file_input_str;

// Parser stack
extern struct text_parser *g_parser_stack[];
extern int                 g_cur_parser_index;

/// get next lexer token
extern token_type lexer_next(void);


/// Attempts to open a file handle given a relative or full path.
/// Also called by preprocessor #include
///
/// \brief
/// #include "foo.h"    -> cwd = .
/// #include "C:\foo.h" -> cwd = C:/
///
/// Precendence: (higher number = higher precedence)
/// 0. LIBCCL include directory. \see COMPILER_INCLUDE_ENV_VAR
/// 1. Directory of file being parsed
extern struct text_parser*
parser_fopen(_in_ const char *fname,
              _out_ int *size,
              _in_ struct text_parser *parser);

/// Parser entry method
// extern int parser_run(FILE *file_in, char *fname);
extern int parser_run(_in_ struct text_parser *parser);

void parser_test(struct text_parser *parser);


// Text parser functions
extern struct ast_item      *parse_expr(void);
extern struct ast_item      *parse_block(void);
extern struct ast_proto     *parse_proto(enum token_type t);
extern struct ast_func      *parse_def(void);
extern struct ast_item      *parse_call(char *ident);


#ifdef __cplusplus
}
#endif

#endif //LIBPRCO_PARSER_H
