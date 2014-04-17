#pragma once
#include "mpc.h"
#include "lenv.h"
#define LASSERT(args, cond, fmt, ...)                   \
    if (!(cond)) {                                      \
        lval* err = lval_err(fmt, ##__VA_ARGS__);       \
        lval_delete(args);                              \
        return err;                                     \
    }
#define LASSERT_SIZE(args, size, fmt, ...) \
    if (args->count != size) { \
        lval_delete(args); \
        return lval_err(fmt, ##__VA_ARGS__); \
    }
#define LASSERT_NONEMPTY(args, fmt, ...) \
    if (args->cell[0]->count == 0){                           \
        lval_delete(args);                                    \
        return lval_err(fmt, ##__VA_ARGS__);                  \
    }
#define LASSERT_ARG_TYPE(args, pos, expected_type, fmt, ...)      \
    if (args->cell[pos]->type != expected_type) {                 \
        lval_delete(args);                                        \
        return lval_err(fmt, ##__VA_ARGS__);                      \
    }

enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };

typedef lval*(*lbuiltin)(lenv*, lval*);

// Forward declared in lenv.h
struct lval {
    int type;
    long num;

    // Error and Symbol types have string data
    char* err;
    char* sym;

    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;

    // List of lvals
    struct lval** cell;
    int count;
};

lval* lval_num (long x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin func);
lval* lval_lambda(lval* formals, lval* body);

lval* lval_read(mpc_ast_t* t);
void lval_println(lenv* e, lval* v);
void lval_print(lenv* e, lval* v);
void lval_delete(lval* v);
lval* lval_copy(lval* v);

lval* lval_eval(lenv *e, lval* v);
lval* lval_call(lenv* e, lval* f, lval* a);
lval* lval_take(lval* v, int i);
lval* lval_pop(lval* v, int i);
lval* lval_add(lval* v, lval* x);

lval* builtin_list(lenv* e, lval* a);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a);
lval* builtin_len(lenv* e, lval* a);
lval* builtin_init(lenv* e, lval* a);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);
lval* builtin_fun(lenv* e, lval* a);
lval* builtin_gt(lenv* e, lval* a);
lval* builtin_lt(lenv* e, lval* a);
lval* builtin_ge(lenv* e, lval* a);
lval* builtin_le(lenv* e, lval* a);
lval* builtin_eq(lenv* e, lval* a);
lval* builtin_ne(lenv* e, lval* a);
lval* builtin_if(lenv* e, lval* a);
lval* builtin_or(lenv* e, lval* a);
lval* builtin_and(lenv* e, lval* a);
lval* builtin_not(lenv* e, lval* a);

char* ltype_name(int t);
