#include <stdio.h>
#include <stdlib.h>

#include "lval.h"
#include "mpc.h"

lval* lval_num (long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval* lval_err(char* m) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
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
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
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

// Forward declare helper function
void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);

        if (i != (v->count - 1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print(lval *v) {
    switch (v->type) {
    case LVAL_NUM:   printf("%li", v->num); break;
    case LVAL_ERR:   printf("Error: %s", v->err); break;
    case LVAL_SYM:   printf("%s", v->sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
    }
}
void lval_println (lval* v) { lval_print(v); putchar('\n'); }

lval* lval_eval_sexpr(lval* v) {
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
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
    if (f->type != LVAL_SYM) {
        lval_delete(f);
        lval_delete(v);
        return lval_err("S-expression does not start with a symbol");
    }
    lval* result = builtin(v, f->sym);
    lval_delete(f);
    return result;
}

lval* lval_eval(lval* v) {
    if (v->type == LVAL_SEXPR) {
        return lval_eval_sexpr(v);
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

lval* builtin_op(lval* v, char* op) {
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type != LVAL_NUM) {
            lval_delete(v);
            return lval_err("Cannot do operator on a non-number");
        }
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
                x = lval_err("Division by Zero");
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

lval* builtin_head(lval* a) {
    LASSERT_SIZE(a, 1, "Head function passed too many arguments");
    LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
            "Head function not passed a Qexpr");
    LASSERT_NONEMPTY(a, "Head function passed {}");

    lval* v = lval_take(a, 0);
    while (v->count > 1) {
        lval_pop(v, 1);
    }
    return v;
}

lval* builtin_tail(lval* a) {
    LASSERT_SIZE(a, 1, "Tail function passed too many arguments");
    LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
            "Tail function not passed a Qexpr");
    LASSERT_NONEMPTY(a, "Tail function passed {}");

    lval* v = lval_take(a, 0);
    lval_delete(lval_pop(v, 0));
    return v;
}

lval* builtin_list(lval* a) {
    LASSERT(a, (a->type == LVAL_SEXPR), "List function not passed a Sexpr");
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_eval(lval* a) {
    LASSERT_SIZE(a, 1, "Eval function passed wrong number of arguments");
    LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
            "Eval function not passed a Qexpr");

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

lval* builtin_join(lval* a) {
    for (int i = 0; i < a->count; i++) {
        LASSERT(a, (a->cell[i]->type == LVAL_QEXPR),
                "Function join passed an argument that isn't a Qexpr");
    }

    lval* x = lval_pop(a, 0);
    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    lval_delete(a);
    return x;
}

lval* builtin_cons(lval* a) {
    LASSERT_SIZE(a, 2, "Cons not called with two arguments");
    LASSERT(a, (a->cell[1]->type == LVAL_QEXPR),
            "Second argument to Cons must be a QExpr");

    lval* n = lval_pop(a, 0);
    lval* v = lval_pop(a, 0);

    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    memmove(&v->cell[1], &v->cell[0], sizeof(lval*) * (v->count - 1));
    v->cell[0] = n;
    return v;
}

lval* builtin(lval* a, char* func) {
    if (strcmp("list", func) == 0) {return builtin_list(a);}
    if (strcmp("eval", func) == 0) {return builtin_eval(a);}
    if (strcmp("join", func) == 0) {return builtin_join(a);}
    if (strcmp("head", func) == 0) {return builtin_head(a);}
    if (strcmp("tail", func) == 0) {return builtin_tail(a);}
    if (strcmp("cons", func) == 0) {return builtin_cons(a);}
    if (strstr("+-/*", func)) {return builtin_op(a, func);}
    lval_delete(a);
    return lval_err("Unknown function!");
}
