#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"
#include "lval.h"

#if defined(_WIN64) || defined(_WIN32)
// Windows does 'right' things by default. Write stub functions to handle this
// default behavior
static char buffer [2048]
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
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Qexpr  = mpc_new("qexpr");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Lispy  = mpc_new("lispy");

    mpca_lang(MPC_LANG_DEFAULT,
  "                                                                                        \
    number : /-?[0-9]+/ ;                                                                  \
    symbol : \"cons\" | \"list\" | \"head\" | \"tail\" | \"join\" | \"eval\" | '+' | '-' | '*' | '/' ;\
    sexpr  : '(' <expr>* ')' ;                                                             \
    qexpr  : '{' <expr>* '}' ;                                                             \
    expr   : <number> | <symbol> | <sexpr> | <qexpr> ;                                     \
    lispy  : /^/ <expr>* /$/ ;               \
  ",
              Number, Symbol, Sexpr, Expr, Qexpr, Lispy);
    puts("Lispy Version 0.0.0.0.1");
    puts("Press Ctrl+C to exit\n");

    while (1) {
        char* input = readline("lispy> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            lval* x = lval_eval(lval_read(r.output));
            lval_println(x);
            lval_delete(x);
            mpc_ast_delete(r.output);
        }
        else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
}
