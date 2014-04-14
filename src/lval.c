#include <stdio.h>
#include <stdlib.h>

#include "lval.h"
#include "mpc.h"

char* ltype_name(int t) {
    switch(t) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    default: return "Unknown";
    }
}

lval* lval_num (long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval* lval_err(char* fmt, ...) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;

    va_list va;
    va_start(va, fmt);
    v->err = malloc(512);
    vsnprintf(v->err, 511, fmt, va);
    v->err = realloc(v->err, strlen(v->err) + 1);

    va_end(va);
    return v;
}

lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);

    return v;
}

lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;

    return v;
}

lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;

    return v;
}

lval* lval_fun(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->fun = func;
    return v;
}

void lval_delete(lval* v) {
    switch(v->type) {
    case LVAL_NUM:
        break;
    case LVAL_ERR:
        free(v->err);
        break;
    case LVAL_SYM:
        free(v->sym);
        break;
    case LVAL_QEXPR:
    case LVAL_SEXPR:
        for (int i = 0; i < v->count; i++) {
            lval_delete(v->cell[i]);
        }
        free(v->cell);
        break;
    case LVAL_FUN: break; // don't do anything
    }
    free(v);
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval* lval_read_num(mpc_ast_t* t) {
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) :
        lval_err("invalid number: %s", t->contents);
}

lval* lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        return lval_read_num(t);
    }
    if (strstr(t->tag, "symbol")) {
        return lval_sym(t->contents);
    }
    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) {
        x = lval_sexpr();
    }
    if (strstr(t->tag, "sexpr")) {
        x = lval_sexpr();
    }
    if (strstr(t->tag, "qexpr")) {
        x = lval_qexpr();
    }
    for (int i = 0; i < t->children_num; i++) {
        char* contents = t->children[i]->contents;
        char* tag = t->children[i]->tag;
        if (strcmp(contents, "(") == 0) {continue;}
        if (strcmp(contents, ")") == 0) {continue;}
        if (strcmp(contents, "{") == 0) {continue;}
        if (strcmp(contents, "}") == 0) {continue;}
        if (strcmp(tag, "regex") == 0) {continue;}
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

void lval_expr_print(lenv* e, lval* v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {
        lval_print(e, v->cell[i]);

        if (i != (v->count - 1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print(lenv* e, lval *v) {
    switch (v->type) {
    case LVAL_NUM:   printf("%li", v->num); break;
    case LVAL_ERR:   printf("Error: %s", v->err); break;
    case LVAL_SYM:   printf("%s", v->sym); break;
    case LVAL_SEXPR: lval_expr_print(e, v, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(e, v, '{', '}'); break;
    case LVAL_FUN:
        printf("<function:");
        lval_print(e, lenv_lookup_sym(e, v));
        putchar('>');
        break;
    }
}
void lval_println (lenv* e, lval* v) { lval_print(e, v); putchar('\n'); }

lval* lval_copy(lval* v) {
    lval* x = malloc(sizeof(lval));
    x->type = v->type;

    switch (v->type) {
    case LVAL_NUM:
        x->num = v->num;
        break;
    case LVAL_FUN:
        x->fun = v->fun;
        break;

    case LVAL_ERR:
        x->err = malloc(strlen(v->err) + 1);
        strcpy(x->err, v->err);
        break;
    case LVAL_SYM:
        x->sym = malloc(strlen(v->sym) + 1);
        strcpy(x->sym, v->sym);
        break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
        x->count = v->count;
        x->cell = malloc(sizeof(lval*) * x->count);
        for (int i = 0; i < x->count; i++) {
            x->cell[i] = lval_copy(v->cell[i]);
        }
        break;
    }
    return x;
}

lval* lval_eval_sexpr(lenv* e, lval* v) {
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(e, v->cell[i]);
        if (v->cell[i]->type == LVAL_ERR) {
            return lval_take(v, i);
        }
    }

    if (v->count == 0) {
        return v;
    }
    if (v->count == 1) {
        return lval_take(v, 0);
    }

    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_FUN) {
        lval_delete(v);
        lval_delete(f);
        return lval_err("S-expression should start with a %s not a %s",
                        ltype_name(LVAL_FUN), ltype_name(f->type));
    }
    lval* result = f->fun(e, v);
    lval_delete(f);
    return result;
}

lval* lval_eval(lenv* e, lval* v) {
    if (v->type == LVAL_SYM) {
        lval* x = lenv_get(e, v);
        lval_delete(v);
        return x;
    }
    if (v->type == LVAL_SEXPR) {
        return lval_eval_sexpr(e, v);
    }
    return v;
}

lval* lval_pop(lval* v, int i) {
    lval* x = v->cell[i];
    /* Shift the memory following the item at "i" over the top of it */
    memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval*) * (v->count - i - 1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_delete(v);
    return x;
}

lval* lval_join(lval* x, lval* y) {
    while (y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }
    lval_delete(y);
    return x;
}

lval* builtin_op(lenv* e, lval* v, char* op) {
    for (int i = 0; i < v->count; i++) {
        LASSERT(v, (v->cell[i]->type == LVAL_NUM),
                "Cannot do operator on a non-number: %s",
                ltype_name(v->cell[i]->type))
    }

    lval* x = lval_pop(v, 0);
    if ((strcmp(op, "-") == 0) && v->count == 0) {
        x->num = -1 * x->num;
    }

    while (v->count > 0) {
        lval* y = lval_pop(v, 0);

        if (strcmp(op, "+") == 0) {
            x->num += y->num;
        }
        if (strcmp(op, "-") == 0) {
            x->num -= y->num;
        }
        if (strcmp(op, "*") == 0) {
            x->num *= y->num;
        }
        if (strcmp(op, "/") == 0) {
            if (y->num == 0) {
                lval_delete(x);
                lval_delete(y);
                return lval_err("Division by Zero");
            }
            else {
                x->num = x->num / y->num;
            }
        }
        lval_delete(y);
    }
    lval_delete(v);
    return x;
}

lval* builtin_add(lenv* e, lval* a) { return builtin_op(e, a, "+"); }
lval* builtin_sub(lenv* e, lval* a) { return builtin_op(e, a, "-"); }
lval* builtin_mul(lenv* e, lval* a) { return builtin_op(e, a, "*"); }
lval* builtin_div(lenv* e, lval* a) { return builtin_op(e, a, "/"); }

lval* builtin_head(lenv* e, lval* a) {
    LASSERT_SIZE(a, 1, "Head function passed too many arguments");
    LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
            "Head function requires a %s not a %s",
            ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));
    LASSERT_NONEMPTY(a, "Head function passed {}");

    lval* v = lval_take(a, 0);
    while (v->count > 1) {
        lval_pop(v, 1);
    }
    return v;
}

lval* builtin_tail(lenv* e, lval* a) {
    LASSERT_SIZE(a, 1, "Tail function passed too many arguments");
    LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
            "Tail function requires a %s not a %s",
            ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));
    LASSERT_NONEMPTY(a, "Tail function passed {}");

    lval* v = lval_take(a, 0);
    lval_delete(lval_pop(v, 0));
    return v;
}

lval* builtin_list(lenv* e, lval* a) {
    LASSERT(a, (a->type == LVAL_SEXPR),
            "List function requires a %s not a %s",
            ltype_name(LVAL_SEXPR), ltype_name(a->type));
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_eval(lenv* e, lval* a) {
    LASSERT_SIZE(a, 1, "Eval function passed wrong number of arguments");
    LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
            "Eval function requires a %s not a %s",
            ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval* builtin_join(lenv* e, lval* a) {
    for (int i = 0; i < a->count; i++) {
        LASSERT(a, (a->cell[i]->type == LVAL_QEXPR),
                "Join argument %d is not a %s. It is a %s",
                i + 1, ltype_name(LVAL_QEXPR), ltype_name(a->cell[i]->type));
    }

    lval* x = lval_pop(a, 0);
    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    lval_delete(a);
    return x;
}

lval* builtin_cons(lenv* e, lval* a) {
    LASSERT_SIZE(a, 2, "Cons not called with two arguments");
    LASSERT(a, (a->cell[1]->type == LVAL_QEXPR),
            "Second argument to Cons must be a %s not a %s",
            ltype_name(LVAL_QEXPR), ltype_name(a->cell[1]->type));

    lval* n = lval_pop(a, 0);
    lval* v = lval_pop(a, 0);

    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    memmove(&v->cell[1], &v->cell[0], sizeof(lval*) * (v->count - 1));
    v->cell[0] = n;
    return v;
}

lval* builtin_len(lenv* e, lval* a) {
    LASSERT_SIZE(a, 1, "len can only be called with one argument");
    LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
            "len requires a %s not a %s",
            ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));

    lval* v = lval_num(a->cell[0]->count);
    lval_delete(a);
    return v;
}

lval* builtin_init(lenv* e, lval* a) {
    LASSERT_SIZE(a, 1, "init can only be called with one argument");
    LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
            "init requires a %s not a %s",
            ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));

    lval* v = lval_take(a, 0);
    lval_delete(lval_pop(v, v->count - 1));
    return v;
}

lval* builtin_def(lenv* e, lval* a) {
    LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
            "def requires a %s not a %s",
            ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));

    lval* syms = a->cell[0];
    for (int i = 0; i < syms->count; i++) {
        LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
                "%d argument in the def list is not a %s it is a %s",
                i, ltype_name(LVAL_SYM), ltype_name(syms->cell[i]->type));
    }

    LASSERT(a, (syms->count == a->count-1),
            "def cannot define incorrect number of values to symbols");

    for (int i = 0; i < syms->count; i++) {
        lenv_put(e, syms->cell[i], a->cell[i + 1]);
    }
    lval_delete(a);
    return lval_sexpr();
}
