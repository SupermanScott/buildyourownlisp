#pragma once
#include "mpc.h"
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

typedef struct lval {
    int type;
    long num;

    // Error and Symbol types have string data
    char* err;
    char* sym;

    // List of lvals
    struct lval** cell;
    int count;
} lval;

lval* lval_read(mpc_ast_t* t);
void lval_println(lval* v);
void lval_delete(lval* v);
