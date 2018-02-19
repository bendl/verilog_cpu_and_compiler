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

#include <stdio.h>
#include <assert.h>
#include <Shlwapi.h>
#include <stdlib.h>
#include <string.h>

/// Lexer current token
int g_cur_token;

static char line_buf[80];
static int line_buf_pos = 0;

// Parser stack
#define PARSER_MAX_STACK 5
struct text_parser *g_parser_stack[PARSER_MAX_STACK];
int g_cur_parser_index = -1;
#define g_cur_parser() g_parser_stack[g_cur_parser_index]

/// Lexer current ident string
char *ident_str;
static int ch = ' ';

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
                ch          = ' ';
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
        char                    buf_run_dir[MAX_PATH];

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
                // Get current dir from current parser
                // If current parser is null, use executable pwd
                // I.e. pwd + fname
                if (parser == NULL)
                {
                        GetCurrentDirectory(MAX_PATH, new_parser->lpstr_input_dir);
                        PathCombine(new_parser->lpstr_input_fp, new_parser->lpstr_input_dir,
                                    fname);
                        strcpy(new_parser->lpstr_input_dir, new_parser->lpstr_input_fp);
                        PathRemoveFileSpec(new_parser->lpstr_input_dir);
                }
                else
                {
                        // Copy old parser dir to new parser dir
                        strcpy(new_parser->lpstr_input_dir, parser->lpstr_input_dir);
                        PathCombine(new_parser->lpstr_input_fp, new_parser->lpstr_input_dir,
                                    fname);
                }
        }
        else
        {
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
                // Check to see if it exists in the std include path
                char *std_include = new_parser->lpstr_input_dir;
                int ret;

                ret = GetEnvironmentVariable(COMPILER_INCLUDE_ENV_VAR,
                                             new_parser->lpstr_input_dir, MAX_PATH);
                if (ret == 0)
                {
                        dprintf(
                            D_WARN,
                            "WARNING: " COMPILER_INCLUDE_ENV_VAR
                            " environment variable is not set. Cannot search std include.");

                        // Else use previous parser's directory
                        std_include = parser->lpstr_input_dir;
                }

                // Create new path from fname and std_include path
                // TODO: Search each directory in PATH for fname?
                new_parser->lpstr_input_dir = std_include;
                PathCombine(
                    new_parser->lpstr_input_fp,
                    new_parser->lpstr_input_dir,
                    fname);
        }

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
        ch = fgetc(g_cur_parser()->file_input);
        if (ch == '\r' || ch == '\n')
        {
                dprintf(D_INFO, "Parsed line: %s\r\n", line_buf);
                line_buf[0] = 0;
                line_buf_pos = 0;
        }
        else
        {
                line_buf[line_buf_pos++] = ch;
                line_buf[line_buf_pos + 1] = 0;
        }
        return ch;
}

int parse_top_level(void) 
{
        assert("Unimplemented" && 0);
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

        // Push new parser to stack
        parse_result = parser_pushp(parser);
        if (parse_result != R_OK) {
                dprintf(D_ERR, "ERROR: Parser stack limit (%d) reached!\r\n",
                                PARSER_MAX_STACK);
                return parse_result;
        }

        // Initialize lexer
        ch = ' ';
        g_cur_token = lexer_next();

        // lexer loop
        while (1) {
                switch (g_cur_token) {
                case TOK_EOF:
                        g_cur_parser()->i_parse_result = (void *)0;
                        parse_result = g_cur_parser()->i_parse_result;
                        goto parser_run_cleanup;
                
                /*case TOK_PREP_DIRECTIVE:
                        g_cur_parser()->i_parse_result = (void *)parse_pre_directive();
                        break;
                case TOK_EXT:
                        g_cur_parser()->i_parse_result = !(void *)parse_extern();
                        break;
                case TOK_DEF:
                case TOK_CC_CDECL:
                case TOK_CC_STDCALL:
                case TOK_CC_FASTCALL:
                        g_cur_parser()->i_parse_result = !(void *)parse_def();
                        break;
                case ';':
                        g_cur_parser()->i_parse_result = !(void *)parse_semicolon();
                        break;
                        */
                default:
                        g_cur_parser()->i_parse_result = (void *)parse_top_level();
                        break;
                } // end switch

                if (g_cur_parser() == 0 && g_cur_token == TOK_EOF) {
                        // TODO: Fix ^^^^ this!
                        return 0;
                }
                if (g_cur_parser()->i_parse_result != 0) {
                        return g_cur_parser()->i_parse_result;
                }
        }

parser_run_cleanup:
        parser_popp();
        return parse_result;
}

token_type 
lexer_next(void)
{
        assert("Unimplemented" && 0);
}

void
parser_test(struct text_parser *parser)
{
        char c;
        int parse_result;

        // Push new parser to stack
        parse_result = parser_pushp(parser);
        if (parse_result != R_OK) {
                dprintf(D_ERR, "ERROR: Parser stack limit (%d) reached!\r\n",
                        PARSER_MAX_STACK);
                return;
        }

        printf("Parser test\r\n");
        printf("Lexing file: %s\r\n", g_cur_parser()->lpstr_input_fp);

        while((c = lexer_fgetc())) {
                printf("char: %c %d\r\n", c, c);
                if(c == EOF) break;
        }
}