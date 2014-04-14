#pragma once
typedef struct lval lval;
typedef struct lenv lenv;
struct lenv {
    int count;
    char** syms;
    lval** vals;
};

void lenv_delete(lenv* e);
lenv* lenv_new(void);

lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);

void lenv_add_builtins(lenv* e);
lval* lenv_lookup_sym(lenv* e, lval* v);
