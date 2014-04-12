#pragma once
#include "mpc.h"
#define LASSERT(args, cond, err) if (!(cond)) { lval_delete(args); return lval_err(err); }
#define LASSERT_SIZE(args, size, err) if (args->count != size) { lval_delete(args); return lval_err(err);}
#define LASSERT_NONEMPTY(args, err) if (args->cell[0]->count == 0) { lval_delete(args); return lval_err(err);}

enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };

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

lval* lval_num (long x);
lval* lval_err(char* m);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

lval* lval_read(mpc_ast_t* t);
void lval_println(lval* v);
void lval_print(lval* v);
void lval_delete(lval* v);
lval* lval_eval(lval* v);
lval* lval_take(lval* v, int i);
lval* lval_pop(lval* v, int i);
lval* builtin(lval* v, char* op);
lval* lval_add(lval* v, lval* x);
