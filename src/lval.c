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
    case LVAL_STR: return "String";
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
    v->builtin = func;
    return v;
}

lval* lval_str(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->str = malloc(strlen(s) + 1);
    strcpy(v->str, s);
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
    case LVAL_STR:
        free(v->str);
        break;
    case LVAL_QEXPR:
    case LVAL_SEXPR:
        for (int i = 0; i < v->count; i++) {
            lval_delete(v->cell[i]);
        }
        free(v->cell);
        break;
    case LVAL_FUN:
        if (v->builtin == NULL) {
            lenv_delete(v->env);
            lval_delete(v->formals);
            lval_delete(v->body);
        }
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

lval* lval_lambda(lval* formals, lval* body) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;

    v->builtin = NULL;
    v->env = lenv_new();
    v->formals = formals;
    v->body = body;
    return v;
}

lval* lval_read_num(mpc_ast_t* t) {
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) :
        lval_err("invalid number: %s", t->contents);
}

lval* lval_read_str(mpc_ast_t* t) {
    t->contents[strlen(t->contents) - 1] = '\0';
    char* unescaped = malloc(strlen(t->contents) + 1 + 1);
    strcpy(unescaped, t->contents + 1);
    unescaped = mpcf_unescape(unescaped);
    lval* str = lval_str(unescaped);
    free(unescaped);
    return str;
}

lval* lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        return lval_read_num(t);
    }
    if (strstr(t->tag, "symbol")) {
        return lval_sym(t->contents);
    }
    if (strstr(t->tag, "string")) {
        return lval_read_str(t);
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
        if (strstr(t->children[i]->tag, "comment")) { continue; }
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

void lval_print_str(lval* v) {
    char* escaped = malloc(strlen(v->str) + 1);
    strcpy(escaped, v->str);
    escaped = mpcf_escape(escaped);
    printf("\"%s\"", escaped);
    free(escaped);
}

void lval_print(lenv* e, lval *v) {
    switch (v->type) {
    case LVAL_NUM:   printf("%li", v->num); break;
    case LVAL_ERR:   printf("Error: %s", v->err); break;
    case LVAL_SYM:   printf("%s", v->sym); break;
    case LVAL_SEXPR:
        if (v->count) {
            lval_expr_print(e, v, '(', ')');
        }
        else {
            printf("ok");
        }
        break;
    case LVAL_QEXPR: lval_expr_print(e, v, '{', '}'); break;
    case LVAL_STR: lval_print_str(v); break;
    case LVAL_FUN:
        printf("<function:");
        lval_print(e, lenv_lookup_sym(e, v));
        if (v->builtin == NULL) {
            printf("(\\ ");
            lval_print(e, v->formals); putchar(' ');
            lval_print(e, v->body); putchar(')');
        }
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
        x->builtin = v->builtin;
        if (v->builtin == NULL) {
            x->env = lenv_copy(v->env);
            x->formals = lval_copy(v->formals);
            x->body = lval_copy(v->body);
        }
        break;

    case LVAL_ERR:
        x->err = malloc(strlen(v->err) + 1);
        strcpy(x->err, v->err);
        break;
    case LVAL_SYM:
        x->sym = malloc(strlen(v->sym) + 1);
        strcpy(x->sym, v->sym);
        break;
    case LVAL_STR:
        x->str = malloc(strlen(v->str) + 1);
        strcpy(x->str, v->str);
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
        if (f->count) {
            return lval_err("S-expression should start with a %s not a %s",
                            ltype_name(LVAL_FUN), ltype_name(f->type));
            lval_delete(f);
        }
        lval* result = lval_str("ok");
        lval_delete(f);
        return result;
    }
    lval* result = lval_call(e, f, v);
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

lval* lval_call(lenv* e, lval* f, lval* a) {
    if (f->builtin) {
        return f->builtin(e, a);
    }

    int given = a->count;
    int total = f->formals->count;

    while(a->count) {
        if (f->formals->count == 0) {
            lval_delete(a);
            return lval_err("Function passed too many arguments." \
                            " Got %i, Expected %i",
                            given, total);
        }
        lval* sym = lval_pop(f->formals, 0);
        if (strcmp("&", sym->sym) == 0) {
            if (f->formals->count != 1) {
                lval_delete(a);
                return lval_err("Function format invalid. Symbol '&' not " \
                                "followed by single symbol");
            }
            lval* nsym = lval_pop(f->formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_delete(nsym);
            lval_delete(sym);
            break;
        }
        else {
            lval* val = lval_pop(a, 0);

            lenv_put(f->env, sym, val);
            lval_delete(sym);
            lval_delete(val);
        }
    }

    lval_delete(a);
    if (f->formals->count == 0) {
        f->env->par = e;
        return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    }
    else if (strcmp(f->formals->cell[0]->sym, "&") == 0) {
        // Only remaining formals is &xs. Because &xs is optional, this means
        // that this should be evaluated.
        if (f->formals->count != 2) {
            return lval_err("Function format invalid. Symbol '&' not followed "\
                            "by single symbol.");
        }

        // Delete the & symbol
        lval_delete(lval_pop(f->formals, 0));
        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_qexpr();
        lenv_put(f->env, sym, val);
        lval_delete(sym);
        lval_delete(val);
        return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    }

    return lval_copy(f);
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

int lval_eq(lval* x, lval* y) {
    if (x->type != y->type) {
        return 0;
    }
    switch (x->type) {
    case LVAL_NUM:
        return x->num == y->num;
    case LVAL_ERR:
        return (strcmp(x->err, y->err) == 0);
    case LVAL_SYM:
        return (strcmp(x->sym, y->sym) == 0);
    case LVAL_STR:
        return (strcmp(x->str, y->str) == 0);
    case LVAL_FUN:
        if (y->builtin || x->builtin) {
            return y->builtin == x->builtin;
        }
        return lval_eq(y->formals, x->formals) && \
            lval_eq(y->body, x->body);
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        if (x->count != y->count) {
            return 0;
        }
        for (int i = 0; i < x->count; i++) {
            if (lval_eq(x->cell[i], y->cell[i]) == 0) {
                return 0;
            }
        }
        return 1;
    }
    return 0;
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
        if (strcmp(op, "%") == 0) {
            x->num = x->num % y->num;
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
lval* builtin_modulo(lenv* e, lval* a) { return builtin_op(e, a, "%"); }

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

lval* builtin_var(lenv* e, lval* a, char* func) {
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
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], a->cell[i + 1]);
        }
        else if (strcmp(func, "=") == 0) {
            lenv_put(e, syms->cell[i], a->cell[i + 1]);
        }
    }
    lval_delete(a);
    return lval_sexpr();
}

lval* builtin_def (lenv* e, lval* a) {
    return builtin_var(e, a, "def");
}

lval* builtin_put (lenv* e, lval* a) {
    return builtin_var(e, a, "=");
}

lval* builtin_lambda(lenv* e, lval* a) {
    LASSERT_SIZE(a, 2, "Lambda not passed two arguments. Passed %d", a->count);
    LASSERT_ARG_TYPE(a, 0, LVAL_QEXPR,
                     "Lambda first argument isn't %s. It is %s",
                     ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));
    LASSERT_ARG_TYPE(a, 1, LVAL_QEXPR,
                     "Lambda second argument isn't %s. It is %s",
                     ltype_name(LVAL_QEXPR), ltype_name(a->cell[1]->type));
    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
                "Cannot define non-symbol. Got %s, Expected %s.",
                ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
    }

    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    return lval_lambda(formals, body);
}

lval* builtin_fun(lenv* e, lval* a) {
    LASSERT_SIZE(a, 2, "Lambda not passed two arguments. Passed %d", a->count);
    LASSERT_ARG_TYPE(a, 0, LVAL_QEXPR,
                     "Lambda first argument isn't %s. It is %s",
                     ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));
    LASSERT_ARG_TYPE(a, 1, LVAL_QEXPR,
                     "Lambda second argument isn't %s. It is %s",
                     ltype_name(LVAL_QEXPR), ltype_name(a->cell[1]->type));
    lval* args = lval_pop(a, 0);
    lval* name = lval_pop(args, 0);
    lval* body = lval_pop(a, 0);

    lval* func = lval_lambda(args, body);
    lenv_def(e, name, func);

    lval_delete(a);
    return func;
}

lval* builtin_ord(lenv* e, lval* a, char* op) {
    LASSERT_SIZE(a, 2, "Executing %s requires two %s",
                 op, ltype_name(LVAL_NUM));
    LASSERT_ARG_TYPE(a, 0, LVAL_NUM,
                     "First argument to %s need to be a %s not a %s",
                     op, ltype_name(LVAL_NUM), ltype_name(a->cell[0]->type));
    LASSERT_ARG_TYPE(a, 1, LVAL_NUM,
                     "Second argument to %s need to be a %s not a %s",
                     op, ltype_name(LVAL_NUM), ltype_name(a->cell[1]->type));
    int r;
    if (strcmp(op, ">")  == 0) { r = (a->cell[0]->num >  a->cell[1]->num); }
    if (strcmp(op, "<")  == 0) { r = (a->cell[0]->num <  a->cell[1]->num); }
    if (strcmp(op, ">=") == 0) { r = (a->cell[0]->num >= a->cell[1]->num); }
    if (strcmp(op, "<=") == 0) { r = (a->cell[0]->num <= a->cell[1]->num); }
    lval_delete(a);
    return lval_num(r);
}

lval* builtin_gt(lenv* e, lval* a) { return builtin_ord(e, a, ">");  }
lval* builtin_lt(lenv* e, lval* a) { return builtin_ord(e, a, "<");  }
lval* builtin_ge(lenv* e, lval* a) { return builtin_ord(e, a, ">="); }
lval* builtin_le(lenv* e, lval* a) { return builtin_ord(e, a, "<="); }

lval* builtin_cmp(lenv* e, lval* a, char* op) {
    LASSERT_SIZE(a, 2, "Comparison operator %s requires two arguments not %i",
                 op, a->count);
    int r = -1;
    if (strcmp(op, "==") == 0) {
        r = lval_eq(a->cell[0], a->cell[1]);
    }
    else if (strcmp(op, "!=") == 0) {
        r = ! lval_eq(a->cell[0], a->cell[1]);
    }

    lval* val = NULL;
    if (r == -1) {
        val = lval_err("Comparison operator %s isn't defined", op);
    }
    else {
        val = lval_num(r);
    }

    lval_delete(a);
    return val;
}

lval* builtin_eq(lenv* e, lval* a) { return builtin_cmp(e, a, "=="); }
lval* builtin_ne(lenv* e, lval* a) { return builtin_cmp(e, a, "!="); }

lval* builtin_if(lenv* e, lval* a) {
    LASSERT_SIZE(a, 3, "If requires three arguments. One for condition, one "\
                 "for if condition is True and finally one for if condition "\
                 "is false");
    LASSERT_ARG_TYPE(a, 0, LVAL_NUM,
                     "First argument to If must evaluate to a %s",
                     ltype_name(LVAL_NUM));
    LASSERT_ARG_TYPE(a, 1, LVAL_QEXPR,
                     "Second argument to If must be a %s",
                     ltype_name(LVAL_QEXPR));
    LASSERT_ARG_TYPE(a, 2, LVAL_QEXPR,
                     "Third argument to If must be a %s",
                     ltype_name(LVAL_QEXPR));
    a->cell[1]->type = LVAL_SEXPR;
    a->cell[2]->type = LVAL_SEXPR;

    lval* x = NULL;
    if (a->cell[0]->num) {
        x = lval_eval(e, lval_pop(a, 1));
    }
    else {
        x = lval_eval(e, lval_pop(a, 2));
    }
    lval_delete(a);
    return x;
}

lval* builtin_or(lenv* e, lval* a) {
    for (int i = 0; i < a->count; i++) {
        LASSERT_ARG_TYPE(a, i, LVAL_NUM,
                         "Condition %i does not evaluate to a %s but a %s",
                         i, ltype_name(LVAL_NUM), ltype_name(a->cell[i]->type));
        if (a->cell[i]->num) {
            lval_delete(a);
            return lval_num(1);
        }
    }
    lval_delete(a);
    return lval_num(0);
}

lval* builtin_and(lenv* e, lval* a) {
    for (int i = 0; i < a->count; i++) {
        LASSERT_ARG_TYPE(a, i, LVAL_NUM,
                         "Condition %i does not evaluate to a %s but a %s",
                         i, ltype_name(LVAL_NUM), ltype_name(a->cell[i]->type));
        if (a->cell[i]->num == 0) {
            lval_delete(a);
            return lval_num(0);
        }
    }
    lval_delete(a);
    return lval_num(1);
}

lval* builtin_not(lenv* e, lval* a) {
    LASSERT_SIZE(a, 1, "Not only accepts one argument");
    LASSERT_ARG_TYPE(a, 0, LVAL_NUM,
                     "First argument to not must be a %s not a %s",
                     ltype_name(LVAL_NUM), ltype_name(a->cell[0]->type));

    lval* v = NULL;
    if (a->cell[0]->num > 0) {
        v = lval_num(0);
    }
    else {
        v = lval_num(1);
    }
    lval_delete(a);
    return v;
}

lval* builtin_load(lenv* e, lval* a) {
    LASSERT_SIZE(a, 1, "Load only accepts one argument");
    LASSERT_ARG_TYPE(a, 0, LVAL_STR,
                     "First argument to load must be a %s not a %s",
                     ltype_name(LVAL_STR), ltype_name(a->cell[0]->type));
    mpc_result_t r;
    if (mpc_parse_contents(a->cell[0]->str, Lispy, &r)) {
        lval* expr = lval_read(r.output);
        while(expr->count) {
            lval* x = lval_eval(e, lval_pop(expr, 0));
            if (x->type == LVAL_ERR) {
                lval_println(e, x);
            }
            lval_delete(x);
        }

        lval_delete(expr);
        lval_delete(a);
        return lval_sexpr();
    }
    else {
        char* err_msg = mpc_err_string(r.error);
        mpc_err_delete(r.error);

        lval* err = lval_err("Could not load Library %s", err_msg);
        free(err_msg);
        lval_delete(a);

        return err;
    }
}

lval* builtin_print(lenv* e, lval* a) {
    for (int i = 0; i < a->count; i++) {
        lval_print(e, a->cell[i]);
        putchar(' ');
    }
    putchar('\n');
    lval_delete(a);
    return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
    LASSERT_SIZE(a, 1, "Error function requires exactly one argument");
    LASSERT_ARG_TYPE(a, 0, LVAL_STR, "Error function requires a %s not a %s",
                     ltype_name(LVAL_STR), ltype_name(a->cell[0]->type));

    lval* err = lval_err(a->cell[0]->str);
    lval_delete(a);
    return err;
}
