//
// Created by BDL on 19/02/2018.
//

#include "parser.h"
#include "dbug.h"
#include "types.h"
#include "adt/ast.h"
#include "adt/list.h"

// Parser shouldnt need to know about target architecture
#include "arch/target.h"
#include "module.h"

#include <stdio.h>
#include <assert.h>
#include <Shlwapi.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/// Lexer current token
int g_cur_token;

/// Lexer current number
int g_num_val;

static char line_buf[80];
static int line_buf_pos = 0;

// Parser stack
#define PARSER_MAX_STACK 5
struct text_parser *g_parser_stack[PARSER_MAX_STACK];
int g_cur_parser_index = -1;
#define g_cur_parser() g_parser_stack[g_cur_parser_index]

/// Lexer current ident string
char *ident_str;
static int g_ch = ' ';

struct list_item *g_locals;     //< Linked list of current local decls
struct list_item *g_params;     //< Linked list of current prototype params
struct list_item *g_resv_words; //< Linked list of reserved words

// TODO: Use or remove
#define eat() \
        g_cur_parser = lexer_next();

// TODO: Use or remove
#define lexer_tok_to_string(t) TOK_STR[t]

static void parser_free(_in_ struct text_parser *parser)
{
        fclose  (parser->file_input);
        free    (parser->lpstr_input_dir);
        free    (parser->lpstr_input_fp);
        free    (parser);
}

int parser_pushp(_inout_ struct text_parser *parser)
{
        g_cur_parser_index++;
        if (g_cur_parser_index > PARSER_MAX_STACK - 1) {
                // Set parser index back to last one, to prevent out of bounds
                // access on function return
                g_cur_parser_index = PARSER_MAX_STACK - 1;
                return PARSER_ERROR_STACK;
        }

        g_parser_stack[g_cur_parser_index] = parser;
        return 0;
}

void parser_popp()
{
        // Free parser resources
        parser_free(g_cur_parser());
        g_parser_stack[g_cur_parser_index--] = NULL;

        // Trigger lexer on the previous parser
        if (g_cur_parser_index >= 0) {
                g_ch          = ' ';
                g_cur_token = ' ';
                g_cur_token = lexer_next();
        }
}

/// Determine relative or full path
static int file_exists(const char *fname)
{
        // TODO: Create cross-platform implementation
        return PathFileExists(fname);
}


// TODO: Make work for linux
struct text_parser*
parser_fopen(_in_ const char *fname, 
             _out_ int *size,
             _in_ struct text_parser *parser)
{
        struct text_parser      *new_parser;

        dprintf(D_INFO, "parser_fopen: %s\r\n", fname);

        new_parser = calloc(1, sizeof(struct text_parser));
        new_parser->lpstr_input_fp =  malloc(MAX_PATH);
        new_parser->lpstr_input_dir = malloc(MAX_PATH);

#ifdef _WIN32
        // Hack to replace linux directory separators / with \
        // TODO: Use cross-platform solution for handling directory filepaths
        {
                char *c = fname;
                while (*c != 0)
                {
                        if (*c == '/')
                                *c = '\\';
                        c++;
                }
        }
#endif

        if (PathIsRelative(fname))
        {
                dprintf(D_INFO, "Path is relative\r\n");
                // Get current dir from current parser
                // If current parser is null, use executable pwd
                // I.e. pwd + fname
                if (parser == NULL) {
                        GetCurrentDirectory(MAX_PATH, new_parser->lpstr_input_dir);
                        dprintf(D_INFO, "dir: %s\r\n", new_parser->lpstr_input_dir);
                        dprintf(D_INFO, "fname: %s\r\n", fname);
                        PathCombine(new_parser->lpstr_input_fp,
                                new_parser->lpstr_input_dir, fname);
                        strcpy(new_parser->lpstr_input_dir, new_parser->lpstr_input_fp);
                        PathRemoveFileSpec(new_parser->lpstr_input_dir);
                        dprintf(D_INFO, "fp: %s\r\n", new_parser->lpstr_input_fp);
                        dprintf(D_INFO, "Parser null\r\n");
                }
                else
                {
                        // Copy old parser dir to new parser dir
                        strcpy(new_parser->lpstr_input_dir,
                               parser->lpstr_input_dir);
                        PathCombine(new_parser->lpstr_input_fp,
                                    new_parser->lpstr_input_dir,
                                    fname);
                }

                dprintf(D_INFO, "Origin path: %s\r\n", fname);
                dprintf(D_INFO, "New path: %s\r\n", new_parser->lpstr_input_fp);
        } else {
                // Extract dir from fname
                // TODO: Pass size to this function
                int fname_size = strlen(fname) + 1;
                strcpy(new_parser->lpstr_input_fp, fname);
                strcpy(new_parser->lpstr_input_dir, fname);
                PathRemoveFileSpec(new_parser->lpstr_input_dir);
        }

        if (!file_exists(new_parser->lpstr_input_fp))
        {
                // File doesn't exist relative to current file
                // Check to see if id exists in the std include path
                char *std_include = new_parser->lpstr_input_dir;
                int ret;

                dprintf(D_INFO, "File doesnt exist!\r\n");

                ret = GetEnvironmentVariable(COMPILER_INCLUDE_ENV_VAR,
                                             new_parser->lpstr_input_dir, MAX_PATH);
                if (ret == 0)
                {
                        dprintf(
                            D_WARN,
                            "WARNING: " COMPILER_INCLUDE_ENV_VAR
                            " environment variable is not set. Cannot search std include.\r\n");

                        // Else use previous parser's directory
                        if(parser == NULL) {
                                std_include = new_parser->lpstr_input_dir;
                        } else {
                                std_include = parser->lpstr_input_dir;
                        }
                }

                // Create new path from fname and std_include path
                // TODO: Search each directory in PATH for fname?
                new_parser->lpstr_input_dir = std_include;
                PathCombine(
                    new_parser->lpstr_input_fp,
                    new_parser->lpstr_input_dir,
                    fname);
        }


        dprintf(D_INFO, "New path exists: %s\r\n", new_parser->lpstr_input_fp);

// fname exists
l_fname_exists:
        new_parser->file_input = fopen(new_parser->lpstr_input_fp, "r");
        if (!new_parser->file_input)
        {
                dprintf(D_ERR,
                        "ERROR: parser_fopen: Cannot find file: %s!\r\n",
                        new_parser->lpstr_input_fp);
                return NULL;
        }

        if (size != NULL)
        {
                fseek(new_parser->file_input, 0L, SEEK_END);
                *size = ftell(new_parser->file_input);
                fseek(new_parser->file_input, 0L, SEEK_SET);
        }

        return new_parser;
}

static void 
parser_add_resv(char *pstr_name, int dt_size, token_type tok)
{
        struct resv_word *w;
        struct list_item *ll;

        ll            = calloc(1, sizeof(*ll));
        w             = calloc(1, sizeof(*w));
        w->lpstr_name = pstr_name;
        w->dt_size    = dt_size;
        w->tok        = tok;
        ll->value     = w;

        g_resv_words = append_ll_item_head(g_resv_words, w);
}

static char lexer_fgetc()
{
        g_ch = fgetc(g_cur_parser()->file_input);
        if (g_ch == '\r' || g_ch == '\n')
        {
                dprintf(D_INFO, "Parsed line: %s\r\n", line_buf);
                line_buf[0] = 0;
                line_buf_pos = 0;
        }
        else
        {
                line_buf[line_buf_pos++] = g_ch;
                line_buf[line_buf_pos + 1] = 0;
        }
        return g_ch;
}

int parse_top_level(void) 
{
        assert("Unimplemented" && 0);
        return 1;
}

int parser_run(_in_ struct text_parser *parser)
{
        // Initialize parser
        int parse_result = 0;

        // Declare reserved words
        // Todo, datasizes should be target independant
        parser_add_resv("int",          dtINT,  TOK_VARIABLE);
        parser_add_resv("var",          dtINT,  TOK_VARIABLE);
        parser_add_resv("def",          0,      TOK_DEF);
        parser_add_resv("ext",          0,      TOK_EXT);
        parser_add_resv("if",           0,      TOK_IF);
        parser_add_resv("for",          0,      TOK_FOR);
        parser_add_resv("else",         0,      TOK_ELSE);
        parser_add_resv("ret",          0,      TOK_RET);
        parser_add_resv("call",         0,      TOK_CALL);

        // Calling convention words
        parser_add_resv("__stdcall",    0,      TOK_CC_STDCALL);
        parser_add_resv("__cdecl",      0,      TOK_CC_CDECL);
        parser_add_resv("__fastcall",   0,      TOK_CC_FASTCALL);

        // PORT words
        parser_add_resv("PORT1",        0,      TOK_PORT_PORT1);
        parser_add_resv("UART1",        0,      TOK_PORT_UART1);


        // Push new parser to stack
        parse_result = parser_pushp(parser);
        if (parse_result != R_OK) {
                dprintf(D_ERR, "ERROR: Parser stack limit (%d) reached!\r\n",
                                PARSER_MAX_STACK);
                return parse_result;
        }

        // Initialize lexer
        g_ch = ' ';
        g_cur_token = lexer_next();

        // lexer loop
        while (1) {
                switch (g_cur_token) {
                case TOK_EOF:
                        g_cur_parser()->parse_result = 0;
                        parse_result = g_cur_parser()->parse_result;
                        goto parser_run_cleanup;
                        break;

                case TOK_DEF:
                case TOK_CC_CDECL:
                case TOK_CC_STDCALL:
                case TOK_CC_FASTCALL:
                        g_cur_parser()->parse_result = !parse_def();
                        break;

                default:
                        g_cur_parser()->parse_result = parse_top_level();
                        break;
                } // end switch

                if (g_cur_parser() == 0 && g_cur_token == TOK_EOF) {
                        // TODO: Fix ^^^^ this!
                        return 0;
                }
                if (g_cur_parser()->parse_result != 0) {
                        return g_cur_parser()->parse_result;
                }
        }

parser_run_cleanup:
        parser_popp();
        return parse_result;
}

token_type 
lexer_next(void)
{
        struct list_item *resv_it;
        char buf[32];
        int i = 0;

        assert(g_cur_parser());
        assert(g_cur_parser()->file_input);
        dprintf(D_INFO, "Lexing: %s\r\n", g_cur_parser()->lpstr_input_fp);

        // Skip whitespace
        while(isspace(g_ch)) lexer_fgetc();
        dprintf(D_INFO, "C: %c %d\r\n", g_ch, g_ch);

        // Switch Single character operators
        switch(g_ch) {
        case '+': lexer_fgetc(); return TOK_PLUS;
        case '(': lexer_fgetc(); return TOK_LBRACE;
        case ')': lexer_fgetc(); return TOK_RBRACE;
        case ',': lexer_fgetc(); return TOK_COMMA;
        case '{': lexer_fgetc(); return TOK_LCBRACE;
        case '}': lexer_fgetc(); return TOK_RCBRACE;
        case '=': lexer_fgetc(); return TOK_ASSIGNMENT;
        case EOF: return TOK_EOF;
        }

        // It could be a string, so loop over until non alpha character reach
        if(isalpha(g_ch)) {
                i = 0;
                do {
                        buf[i++] = g_ch;
                        lexer_fgetc();
                }
                while(isalnum(g_ch));
                // Null terminate the string buffer
                buf[i] = 0;

                // Compare buf with reserved words
                resv_it = g_resv_words;
                list_for_each(resv_it) {
                        struct resv_word *w = (struct resv_word*)resv_it->value;
                        dprintf(D_PARSE, "Comparing '%s' to resv '%s'\r\n",
                                buf, w->lpstr_name);

                        if(strcmp(buf, w->lpstr_name) == 0) {
                                dprintf(D_PARSE, "Resv word found! %s\r\n", w->lpstr_name);
                                g_cur_token = w->tok;
                                return w->tok;
                        }
                }

                // It's not a resv word so id must be an ident
                ident_str = calloc(strlen(buf+1), sizeof(char));
                strcpy(ident_str, buf);
                return TOK_ID;
        }
        // Not an string, so try number
        else if(isdigit(g_ch)) {
                i = 0;
                do {
                        buf[i++] = g_ch;
                        lexer_fgetc();
                } while(isdigit(g_ch));
                // Null terminate the string number buffer
                buf[i] = 0;

                g_num_val = atoi(buf);
                return TOK_NUM;
        }
        // If its an semi-colon, ignore id
        else if (g_ch == ';') {
                lexer_fgetc();
                return lexer_next();
        } else {
                dprintf(D_ERR, "Unknown character: %d %c\r\n", g_ch, g_ch);
                return TOK_ERROR;
        }
}

enum token_type lexer_token(void)
{
        return (enum token_type)g_cur_token;
}

enum token_type lexer_match_next(enum token_type t)
{
        if(lexer_token() != t) {
                dprintf(D_ERR, "Unexpected token! %d %c\r\n", g_cur_token);
                return TOK_ERROR;
        }
        return (enum token_type)(g_cur_token = lexer_next());
}

enum token_type
lexer_match_opt(enum token_type t)
{
        if(lexer_match(t)) {
                g_cur_token = lexer_next();
        }

        return (enum token_type)g_cur_token;
}

int lexer_match(enum token_type t)
{
        return (lexer_token() == t);
}

int lexer_match_req(enum token_type t)
{
        if(!lexer_match(t)) {
                dprintf(D_ERR, "Required text not found! %d %c\r\n",
                        g_cur_token, g_cur_token);
                return TOK_ERROR;
        }

        return R_OK;
}


struct ast_proto *parse_proto(enum token_type t)
{
        struct list_item *args;
        struct list_item *arg;
        struct ast_lvar *lvarg;
        struct ast_lvar *parg;
        struct ast_proto *proto;
        struct ast_proto *proto_it;

        char *fn_name;
        int argc = 0;
        int pargc = 0;

        args = calloc(1, sizeof(*args));

        // def ident (
        lexer_match_opt(TOK_DEF);
        lexer_match_req(TOK_ID);
        fn_name = ident_str;
        lexer_match_next(TOK_ID);
        lexer_match_next(TOK_LBRACE);

        // args, arg1
        lexer_match_opt(TOK_VARIABLE);
        while(lexer_match(TOK_ID) || lexer_match(TOK_VARIABLE)) {
                lexer_match_opt(TOK_VARIABLE);
                lvarg = new_lvar(new_var(ident_str, dtINT));
                append_ll_item(args, lvarg);
                argc++;

                // TODO: make lexer_next() set g_cur_token
                g_cur_token = lexer_next();
                lexer_match_opt(TOK_COMMA);
        }

        pargc = argc + 1;
        arg = args;
        list_for_each(arg) {
                if(!arg->value) break;
                parg = (struct ast_lvar*)arg->value;
                parg->bp_offset = pargc * 1;
                pargc--;
                dprintf(D_INFO, "Parsed proto arg: %s offset %x\r\n",
                        parg->var->name, parg->bp_offset);
        }

        // Eat closing )
        lexer_match_next(TOK_RBRACE);

        // Prototype finished.
        // Now check to see if we already have a matching one in the module
        proto_it = get_g_module()->prototypes;
        list_for_each(proto_it) {
                if(strcmp(proto_it->name, fn_name) == 0) {
                        if(t == TOK_EXT) {
                                return proto_it;
                        } else {
                                dprintf(D_ERR, "Duplication of prototype: %s\r\n",
                                        fn_name);
                                return NULL;
                        }
                }
        }

        g_params = args;

        proto = new_proto(fn_name, args, argc);
        // Move new proto to front of the ll
        proto->next = get_g_module()->prototypes;
        get_g_module()->prototypes = proto;
        return proto;
}

struct ast_func *parse_def(void)
{
        struct ast_func  *ret;
        int num_lvars =  0;

        struct ast_item  *body;
        struct ast_proto *proto;

        // def ident ( args, ... )
        proto = parse_proto(TOK_DEF);

        lexer_match_next(TOK_LCBRACE);
        body = parse_block();
        // TODO: cleanup
        if(!body) return NULL;

        lexer_match_next(TOK_RCBRACE);

        if(!proto || !body) return NULL;

        ret = new_func(proto, body);
        ret->next = get_g_module()->functions;
        ret->locals = g_locals;
        get_g_module()->functions = ret;


        struct list_item *l = ret->locals;
        struct ast_lvar *v;
        list_for_each(l) {
                v = l->value;
                if(!v) break;
                dprintf(D_INFO, "FUNC: %s\tVAR: '%s' %d\r\n",
                        ret->proto->name,
                        v->var->name,
                        v->bp_offset);
        }

        // Finished parsing,
        // clear global lists
        g_locals = g_params = NULL;

        dprintf(D_INFO, "Parsed def: %s argc: %d\r\n",
                proto->name, proto->argc);

        if(strcmp(proto->name, "main") == 0) {
                get_g_module()->entry = ret;
        }

        return ret;
}

struct list_item *add_var_to_scope(struct list_item *scope,
        struct ast_lvar *v)
{
        struct list_item *old_scope;
        if(scope == NULL) {
                scope = calloc(1, sizeof(*scope));
        }

        if(scope->value == NULL) {
                scope->value = v;
        } else {
                old_scope = scope;
                scope = calloc(1, sizeof(*scope));
                scope->value = v;
                scope->next = old_scope;
        }

        return scope;
}

struct ast_lvar *
check_str_in_scope(struct list_item *scope, char *ident)
{
        struct ast_lvar *vc;
        list_for_each(scope) {
                vc = (struct ast_lvar*)scope->value;
                if(!vc)
                        return NULL;

                if(strcmp(ident, vc->var->name) == 0)
                        return vc;
        }
        return NULL;
}

struct ast_lvar *get_var(char *ident)
{
        struct ast_lvar *v;

        // Check if v is a parameter to the function
        v = check_str_in_scope(g_params, ident);
        if(v) return v;

        // Must be a local variable
        return check_str_in_scope(g_locals, ident);
}

int check_call(struct ast_call *call)
{
        // Check call to prototypes
        struct ast_proto *p = get_g_module()->prototypes;

        list_for_each(p) {
                if (strcmp(call->callee, p->name) == 0) {
                        // number of args must match
                        // Ignore data types as we dont really have any
                        if ((call->argc == p->argc)) {
                                call->proto = p;
                                return true;
                        }
                        // bad number of args
                        break;
                }
        }

        return false;
}


struct ast_item *parse_call(char *ident)
{
        struct list_item *args;
        int argc = 0;
        struct ast_call *call;
        struct ast_item *arg_expr;

        args = calloc(1, sizeof(*args));

        lexer_match_next(TOK_LBRACE);
        if(!lexer_match(TOK_RBRACE)) {
                while(1) {
                        arg_expr = parse_expr();
                        if(!arg_expr) {
                                args = NULL;
                                break;
                        }

                        append_ll_item(args, arg_expr);
                        argc++;

                        if(lexer_match(TOK_RBRACE)) break;

                        lexer_match_next(TOK_COMMA);
                }
        }

        lexer_match_next(TOK_RBRACE);

        // Create the call ast item
        call = new_call(ident, args, argc);
        if(check_call(call)) {
                return new_expr(call, AST_CALL);
        } else {
                dprintf(D_ERR, "ERR: Undefined reference to: %s\r\n", ident);
                return NULL;
        }
}

struct ast_item *parse_assignment(char *ident)
{
        struct ast_lvar         *v;
        struct ast_item         *val;
        struct ast_assign       *a;

        lexer_match_next(TOK_ASSIGNMENT);
        v = get_var(ident);
        if(!v) {
                dprintf(D_ERR,
                        "ERR: Var assignment ident '%s' not found in scope\r\n",
                        ident);
                return NULL;
        }

        val = parse_expr();
        if(!val) return NULL;

        a = calloc(1, sizeof(*a));
        a->var = v;
        a->val = val;

        return new_expr(a, AST_ASSIGNMENT);
}

struct ast_item *parse_ident(void)
{
        struct ast_lvar *v;
        char *ident = ident_str;

        lexer_match_next(TOK_ID);

        // call expr: <ident>(...)
        if(lexer_match(TOK_LBRACE)) {
                return parse_call(ident);
        }

        if(lexer_match(TOK_ASSIGNMENT)) {
                return parse_assignment(ident);
        }

        // else its a variable reference
        v = get_var(ident);
        if(!v) {
                dprintf(D_ERR, "Variable '%s' is not in the current scope!\r\n",
                        ident);
                abort();
                return NULL;
                // TODO: fix
        }

        return new_expr(v, AST_VAR_REF);
}

struct ast_item *parse_num(void)
{
        struct ast_num *ret = new_num(g_num_val);
        g_cur_token = lexer_next();
        return new_expr(ret, AST_NUM);
}

struct ast_item *parse_paren(void)
{
        struct ast_item *result;

        // TODO: check lexer_token == (
        g_cur_token = lexer_next();

        // Parse inside the (...)
        result = parse_expr();

        if(!lexer_match(TOK_RBRACE)) {
                dprintf(D_ERR, "ERR: Missing closing ) paren in expr\r\n");
                return NULL;
        }

        g_cur_token = lexer_next();
        return result;

}

struct ast_item *parse_var(void)
{
        char *ident             = NULL;
        struct ast_lvar *v      = NULL;
        struct ast_item *nvar   = NULL;

        dprintf(D_INFO, "Parseing variable\r\n");

        lexer_match_next(TOK_VARIABLE);
        lexer_match(TOK_ID);
        ident = ident_str;
        lexer_match_next(TOK_ID);

        if((v = get_var(ident)) == NULL) {
                v = new_ldecl(new_var(ident, dtINT));
                nvar = new_expr(v, AST_LOCAL_VAR);
                g_locals = add_var_to_scope(g_locals, nvar->expr);
        }

        if(lexer_match(TOK_ASSIGNMENT)) {
                if(nvar) {
                        nvar->next = parse_assignment(v->var->name);
                        if(nvar->next == NULL)
                                return NULL;
                        return nvar;
                } else {
                        nvar = parse_assignment(v->var->name);
                }
        }

        if(nvar == NULL) {
                nvar = new_expr(v, AST_VAR_REF);
        }

        return nvar;
}

struct ast_item *parse_primary(void)
{
        switch(lexer_token()) {
        case TOK_VARIABLE:      return parse_var();
        case TOK_ID:            return parse_ident();
        case TOK_NUM:           return parse_num();
        case TOK_LBRACE:        return parse_paren();
        // TODO: Fix
        case ';':
                g_cur_token = lexer_next();
                return parse_primary();
        default: dprintf(D_ERR, "Unknown primary token: %d %c\r\n",
                         lexer_token(), lexer_token());
                return NULL;
        }
}

int get_tok_prec(void)
{
        switch(lexer_token()) {
        default: return -1;
        case TOK_LESS: return 20;
        case TOK_PLUS: return 20;
        case TOK_SUB: return 20;
        case TOK_STAR: return 40;
        case TOK_DIV: return 40;
        }
}

struct ast_item *
parse_bin_rhs(int min_prec, struct ast_item *lhs)
{
        struct ast_item *rhs;
        int cur_tok_prec;
        int next_tok_prec;
        int bin_op;

        while(1) {
                cur_tok_prec = get_tok_prec();
                if(cur_tok_prec < min_prec) return lhs;
                bin_op = lexer_token();
                // eat bin op
                g_cur_token = lexer_next();
                rhs = parse_primary();
                if(!rhs) return NULL;

                next_tok_prec = get_tok_prec();
                if(cur_tok_prec < next_tok_prec) {
                        rhs = parse_bin_rhs(cur_tok_prec + 1, rhs);
                        if(!rhs) return NULL;
                }
                lhs = new_expr(new_bin(bin_op, lhs, rhs), AST_BIN);
        }
}

struct ast_item *parse_if_expr(void)
{
        struct ast_item *cond = NULL;
        struct ast_item *then = NULL;
        struct ast_item *els  = NULL;

        // if '('
        lexer_match_next(TOK_IF);
        lexer_match_next(TOK_LBRACE);

        // <cond> ')'
        cond = parse_expr();
        lexer_match_next(TOK_RBRACE);

        // '{' <body> '}'
        then = parse_block();
        lexer_match_next(TOK_RCBRACE);

        // else '{' <body> '}'
        if(lexer_match(TOK_ELSE)) {
                lexer_match_next(TOK_ELSE);
                lexer_match_next(TOK_LCBRACE);
                els = parse_block();
                lexer_match_next(TOK_RCBRACE);
        }

        return new_expr(new_if(cond, then, els), AST_IF);
}

struct ast_item *parse_for_expr(void)
{
        struct ast_item *start,
                        *cond,
                        *step,
                        *body;

        lexer_match_next(TOK_FOR);
        lexer_match_next(TOK_LBRACE);
        start = parse_expr();
        cond = parse_expr();
        step = parse_expr();
        lexer_match_next(TOK_RBRACE);

        lexer_match_next(TOK_LCBRACE);
        body = parse_block();
        lexer_match_next(TOK_RCBRACE);

        return new_expr(new_for(start, cond, step, body), AST_FOR);

}

struct ast_item *parse_expr(void)
{
        struct ast_item *lhs;

        if(lexer_match(TOK_IF))
                return parse_if_expr();

        if(lexer_match(TOK_FOR))
                return parse_for_expr();


        lhs = parse_primary();
        if(!lhs) return NULL;
        return parse_bin_rhs(0, lhs);
}


struct ast_item *parse_block(void)
{
        struct ast_item *start = NULL;
        struct ast_item *last  = NULL;

        lexer_match_opt(TOK_LCBRACE);

        while(!lexer_match(TOK_RCBRACE)) {
                struct ast_item *e = parse_expr();
                if(!e) return NULL;

                if(start == NULL) {
                        start = e;
                        while(e->next) e = e->next;
                        last = e;
                } else {
                        last->next = e;
                        while(e->next) e = e->next;
                        last = e;
                }
        }

        return start;
}



struct ast_var *new_var(char *name, int dt)
{
        struct ast_var *ret = calloc(1, sizeof(*ret));
        ret->name       = name;
        ret->dt         = dt;
        return ret;
}

struct ast_lvar *new_lvar(struct ast_var *var)
{
        struct ast_lvar *ret = calloc(1, sizeof(*ret));
        ret->var        = var;
        ret->bp_offset  = 0;
        return ret;
}

struct ast_proto *new_proto(char *name, struct list_item *args, int argc)
{
        struct ast_proto *ret = calloc(1, sizeof(*ret));
        ret->name = name;
        ret->args = args;
        ret->argc = argc;
        return ret;
}

struct ast_item *new_expr(void *expr, enum ast_type type)
{
        struct ast_item *ret = calloc(1, sizeof(*ret));
        ret->expr = expr;
        ret->type = type;
        return ret;
}

struct ast_bin *
new_bin(char op, struct ast_item *lhs, struct ast_item *rhs)
{
        struct ast_bin *ret = calloc(1, sizeof(*ret));
        ret->op  = op;
        ret->lhs = lhs;
        ret->rhs = rhs;
        return ret;
}

struct ast_func *
new_func(struct ast_proto *proto, struct ast_item *body)
{
        struct ast_func *ret = calloc(1, sizeof(*ret));
        ret->proto = proto;
        ret->body = body;
        ret->num_local_vars = 0;
        ret->exit = NULL;
        return ret;
}

struct ast_num *new_num(int num_val)
{
        struct ast_num *ret = calloc(1, sizeof(*ret));
        ret->val = num_val;
        return ret;
}

struct ast_call *
new_call(char *callee, struct list_item *args, int argc)
{
        struct ast_call *ret = calloc(1, sizeof(*ret));
        ret->callee = callee;
        ret->args = args;
        ret->argc = argc;
        return ret;
}

struct ast_if *
new_if(struct ast_item *cond, struct ast_item *then, struct ast_item *els)
{
        struct ast_if *ret = calloc(1, sizeof(*ret));
        ret->cond = cond;
        ret->then = then;
        ret->els = els;
        return ret;
}

struct ast_for *new_for(struct ast_item *start,
                        struct ast_item *cond,
                        struct ast_item *step,
                        struct ast_item *body)
{
        struct ast_for *ret = calloc(1, sizeof(*ret));
        ret->start = start;
        ret->cond = cond;
        ret->step = step;
        return ret;
}

struct ast_lvar  *new_ldecl  (struct ast_var *var)
{
        struct ast_lvar *ret = calloc(1, sizeof(*ret));
        ret->bp_offset = 0;
        ret->var = var;
        return ret;
}