#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"
#include "lval.h"
#include "../config.h"

#if defined(_WIN64) || defined(_WIN32) || HAVE_LIBEDIT == 0
// Windows does 'right' things by default. Write stub functions to handle this
// default behavior
static char buffer [2048];
char* readline (char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer) + 1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy) - 1] = '\0';
    return cpy;
}

void add_history (char* _unused) {};

#elif __APPLE__
#include <readline/readline.h>
#include <readline/history.h>
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

int main (int argc, char** argv) {
    Number = mpc_new("number");
    Symbol = mpc_new("symbol");
    Sexpr  = mpc_new("sexpr");
    Qexpr  = mpc_new("qexpr");
    Expr   = mpc_new("expr");
    Lispy  = mpc_new("lispy");
    String = mpc_new("string");
    Comment = mpc_new("comment");

    mpca_lang(MPC_LANG_DEFAULT,
  "                                                   \
    number : /-?[0-9]+/ ;                             \
    symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;       \
    sexpr  : '(' <expr>* ')' ;                        \
    qexpr  : '{' <expr>* '}' ;                        \
    expr   : <number> | <symbol> | <sexpr> | <qexpr> | <string> | <comment>;\
    string : /\"(\\\\.|[^\"])*\"/ ;                  \
    comment: /;[^\\r\\n]*/ ;                         \
    lispy  : /^/ <expr>* /$/ ;               \
  ",
              Number, Symbol, Sexpr, Expr, Qexpr, Lispy, String, Comment);
    puts("Lispy Version 0.0.0.0.1");
    puts("Press Ctrl+C to exit\n");

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    if (argc == 1) {
        while (1) {
            char* input = readline("lispy> ");
            if (input == NULL) {
                break;
            }
            add_history(input);

            mpc_result_t r;
            if (mpc_parse("<stdin>", input, Lispy, &r)) {
                lval* x = lval_eval(e, lval_read(r.output));
                lval_println(e, x);
                lval_delete(x);
                mpc_ast_delete(r.output);
            }
            else {
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
            }

            free(input);
        }
    }
    else {
        for (int i = 1; i < argc; i++) {
            lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));
            lval* x = builtin_load(e, args);
            if (x->type == LVAL_ERR) { lval_println(e, x); }
            lval_delete(x);
        }
    }

    mpc_cleanup(8, Number, Symbol, Sexpr, Qexpr, Expr, Lispy, String, Comment);
    lenv_delete(e);
}
