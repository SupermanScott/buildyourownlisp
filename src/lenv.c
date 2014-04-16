#include "lenv.h"
#include "lval.h"

lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));
    e->par = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

void lenv_delete(lenv* e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_delete(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

lval* lenv_get(lenv* e, lval* k) {
    LASSERT(k, (k->type == LVAL_SYM),
            "Getting a sym from environment with wrong type");
    for (int i = 0; i < e->count; i++) {
        if (strcmp(k->sym, e->syms[i]) == 0) {
            return lval_copy(e->vals[i]);
        }
    }
    if (e->par) {
        return lenv_get(e->par, k);
    }
    return lval_err("unbound symbol: %s", k->sym);
}

lval* lenv_lookup_sym(lenv* e, lval* v) {
    LASSERT(v, (v->type == LVAL_FUN),
            "Looking up the sym from env with wrong type.");
    for (int i = 0; i < e->count; i++) {
        if (v->builtin == e->vals[i]->builtin) {
            char* sym_name = malloc(strlen(e->syms[i]) + 1);
            strcpy(sym_name, e->syms[i]);
            return lval_sym(sym_name);
        }
    }
    return lval_err("Function is not bound to a symbol");
}

void lenv_put(lenv* e, lval* k, lval* v) {
    if (k->type != LVAL_SYM) {
        return;
    }
    for (int i = 0; i < e->count; i++) {
        if (strcmp(k->sym, e->syms[i]) == 0) {
            lval_delete(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);

    e->vals[e->count - 1] = lval_copy(v);
    e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
    strcpy(e->syms[e->count - 1], k->sym);
}

void lenv_def(lenv* e, lval* k, lval* v) {
    while (e->par) {
        e = e->par;
    }
    lenv_put(e, k, v);
}

void lenv_print(lenv* e) {
    for (int i = 0; i < e->count; i++) {
        char* sym_name = malloc(strlen(e->syms[i]) + 1);
        strcpy(sym_name, e->syms[i]);
        lval_print(e, lval_sym(sym_name));
        printf(": %s\t=>\t", ltype_name(e->vals[i]->type));
        lval_print(e, e->vals[i]);
        putchar('\n');
    }
}

lenv* lenv_copy(lenv* e) {
    lenv* n = malloc(sizeof(lenv));
    n->count = e->count;
    n->par = e->par;

    n->syms = malloc(sizeof(char*) * e->count);
    n->vals = malloc(sizeof(lval*) * e->count);
    for (int i = 0; i < e->count; i++) {
        n->vals[i] = lval_copy(e->vals[i]);

        n->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
    }

    return n;
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);

    lenv_put(e, k, v);
    lval_delete(k);
    lval_delete(v);
}

void lenv_add_builtins(lenv* e) {
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "cons", builtin_cons);
    lenv_add_builtin(e, "len", builtin_len);
    lenv_add_builtin(e, "init", builtin_init);
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "=", builtin_put);
    lenv_add_builtin(e, "\\", builtin_lambda);
    lenv_add_builtin(e, "fun", builtin_fun);

    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
}
