#include "../src/lval.h"
#include "../src/lenv.h"
#include "minunit/minunit.h"

// builtin(lval*, char* op)
MU_TEST(test_lval_cons) {
    lval* v = lval_sexpr();
    lval* x = lval_qexpr();
    lval* y = lval_num(1);

    x = lval_add(x, lval_num(3));
    x = lval_add(x, lval_num(4));

    v = lval_add(v, y);
    v = lval_add(v, x);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_cons(e, v);

    mu_assert(result->type == LVAL_QEXPR,
              "Result from cons should be Qexpr not");
    mu_assert(result->count == 3,
              "Cons should add 1 to the front of the qexpr");
    mu_assert(result->cell[0]->num == 1,
              "Cons should put the number one in front");

    lval_delete(v);
}

MU_TEST(test_lval_list_success) {
    lval* v = lval_sexpr();
    lval* x = lval_num(1);
    lval* y = lval_num(4);

    v = lval_add(v, x);
    v = lval_add(v, y);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_list(e, v);
    mu_assert(result->type == LVAL_QEXPR,
              "List should turn an sexpr into a qexpr");
    lval_delete(v);
}

MU_TEST(test_lval_list_qexpr) {
    lval* v = lval_qexpr();
    lval* x = lval_num(1);
    lval* y = lval_num(4);

    v = lval_add(v, x);
    v = lval_add(v, y);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_list(e, v);
    mu_assert(result->type == LVAL_ERR,
              "When list function is handed a non-sexpr it should error out.");
}

MU_TEST(test_lval_eval_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    lval_add(q, lval_sym("+"));
    lval_add(q, lval_num(1));
    lval_add(q, lval_num(2));

    lval_add(v, q);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_eval(e, v);
    mu_assert(result->type == LVAL_NUM,
              "Eval a qexpr of {+ 1 2} should result in a number");
    mu_assert(result->num == 3,
              "Eval an qexpr of {+ 1 2} should result in 3");
    lval_delete(result);
}

MU_TEST(test_lval_eval_too_large) {
    lval* v = lval_sexpr();
    lval_add(v, lval_sym("+"));
    lval_add(v, lval_num(1));
    lval_add(v, lval_num(2));

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_eval(e, v);
    mu_assert(result->type == LVAL_ERR,
              "When passed more then on Qexpr, eval should error out");
    lval_delete(result);
}

MU_TEST(test_lval_eval_non_qexpr) {
    lval* v = lval_sexpr();
    lval_add(v, lval_sym("+"));

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_eval(e, v);
    mu_assert(result->type == LVAL_ERR,
              "When not passed a qexpr eval should error out");
    lval_delete(result);
}

MU_TEST(test_lval_join_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    lval* y = lval_qexpr();

    q = lval_add(q, lval_num(1));
    q = lval_add(q, lval_num(4));

    y = lval_add(y, lval_num(10));
    y = lval_add(y, lval_num(14));

    v = lval_add(v, q);
    v = lval_add(v, y);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_join(e, v);
    mu_assert(result->type == LVAL_QEXPR,
              "Joining two qexpr should result in a new qexpr");
    mu_assert(result->count == 4,
              "Joining two qexpr of size 2 should result in a qexpr of size 4");
    lval_delete(result);
}

MU_TEST(test_lval_join_non_qexpr) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    lval* y = lval_num(60);

    q = lval_add(q, lval_num(1));
    q = lval_add(q, lval_num(4));

    v = lval_add(v, q);
    v = lval_add(v, y);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_join(e, v);
    mu_assert(result->type == LVAL_ERR,
              "Joining a qexpr to a number should error out");
    lval_delete(result);
}

MU_TEST(test_lval_head_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    q = lval_add(q, lval_num(10));
    v = lval_add(v, lval_add(q, lval_num(1)));

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_head(e, v);
    mu_assert(result->type == LVAL_QEXPR,
              "Calling head on a Qexpr should result in a qexpr");
    mu_assert(result->count == 1,
              "Calling head on a Qexpr should result in a qexpr of size 1");
    mu_assert(result->cell[0]->num == 10,
              "Calling head on qexpr {10 1} should result in 10");
    lval_delete(result);
}

MU_TEST(test_lval_head_too_many_arguments) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    q = lval_add(q, lval_num(10));
    v = lval_add(v, lval_add(q, lval_num(1)));
    v = lval_add(v, lval_num(14));

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_head(e, v);
    mu_assert(result->type == LVAL_ERR,
              "Head takes only one Qexpr not many arguments.");
    lval_delete(result);
}

MU_TEST(test_lval_head_empty_qexpr) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    v = lval_add(v, q);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_head(e, v);
    mu_assert(result->type == LVAL_ERR,
              "Head requires a non empty qexpr.");
    lval_delete(result);
}

MU_TEST(test_lval_tail_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    q = lval_add(q, lval_num(10));
    v = lval_add(v, lval_add(q, lval_num(1)));

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_tail(e, v);
    mu_assert(result->type == LVAL_QEXPR,
              "Calling tail on a Qexpr should result in a qexpr");
    mu_assert(result->count == 1,
              "Calling tail on a Qexpr with size of 2 should result in a qexpr"\
              " of size 1");
    mu_assert(result->cell[0]->num == 1,
              "Calling tail on qexpr {10 1} should result in 1");
    lval_delete(result);
}

MU_TEST(test_lval_tail_too_many_arguments) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    q = lval_add(q, lval_num(10));
    v = lval_add(v, lval_add(q, lval_num(1)));
    v = lval_add(v, lval_num(14));

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_tail(e, v);
    mu_assert(result->type == LVAL_ERR,
              "Tail takes only one Qexpr not many arguments.");
    lval_delete(result);
}

MU_TEST(test_lval_tail_empty_qexpr) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    v = lval_add(v, q);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_tail(e, v);
    mu_assert(result->type == LVAL_ERR,
              "Tail requires a non empty qexpr.");
    lval_delete(result);
}

MU_TEST(test_lval_len_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    v = lval_add(v, lval_add(lval_add(q, lval_num(200)), lval_num(600)));

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_len(e, v);

    mu_assert(result->type == LVAL_NUM,
              "Len should return an number");
    mu_assert(result->num == 2,
              "Len should return 2 for qexpr {200 600}");
    lval_delete(result);
}

MU_TEST(test_lval_len_too_many) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    v = lval_add(v, lval_add(lval_add(q, lval_num(200)), lval_num(600)));
    v = lval_add(v, lval_num(2));

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_len(e, v);

    mu_assert(result->type == LVAL_ERR,
              "Len called with two arguments should error out.");
    lval_delete(result);
}

MU_TEST(test_lval_len_non_qexpr) {
    lval* v = lval_sexpr();
    lval* q = lval_num(1);
    v = lval_add(v, q);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_len(e, v);

    mu_assert(result->type == LVAL_ERR,
              "Len when called without a qexpr should error out");
    lval_delete(result);
}

MU_TEST(test_lval_init_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();

    q = lval_add(q, lval_num(333));
    q = lval_add(q, lval_num(300));
    q = lval_add(q, lval_num(4));

    v = lval_add(v, q);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_init(e, v);
    mu_assert(result->type == LVAL_QEXPR,
              "Init on a qexpr should return another qexpr");
    mu_assert(result->count == 2,
              "Init on a qexpr of size 3 should return a qexpr of size 2");
    mu_assert(result->cell[0]->num == 333,
              "Resulting qexpr should have 333");
    mu_assert(result->cell[1]->num == 300,
              "Resulting qexpr should have 300");
    lval_delete(result);
}

MU_TEST(test_lval_init_too_many_arguments) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();

    q = lval_add(q, lval_num(333));
    q = lval_add(q, lval_num(300));
    q = lval_add(q, lval_num(4));

    v = lval_add(v, q);
    v = lval_add(v, lval_qexpr());

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_init(e, v);
    mu_assert(result->type == LVAL_ERR,
              "Init with two qexpr should result in an error.");
    lval_delete(result);
}

MU_TEST(test_lval_init_non_qexpr) {
    lval* v = lval_sexpr();
    lval* q = lval_num(1);

    v = lval_add(v, q);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_init(e, v);
    mu_assert(result->type == LVAL_ERR,
              "Init called with a number should result in an error.");
    lval_delete(result);
}

MU_TEST(test_lval_def_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    lval* x = lval_sym("x");
    lval* a = lval_num(25);

    q = lval_add(q, x);
    v = lval_add(v, q);
    v = lval_add(v, a);

    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* result = builtin_def(e, v);

    mu_assert(result->type == LVAL_SEXPR,
              "Defining a symbol should result in an empty sexpr");
    lval* val = lenv_get(e, lval_sym("x"));
    mu_assert(val->type == LVAL_NUM,
              "Getting x from environment should result in number");
    mu_assert(val->num == 25,
              "Getting x from environment should match 25");

    lval_delete(result);
    lval_delete(val);
    lenv_delete(e);
}

MU_TEST(test_lval_def_multiple_eval) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    lval* x = lval_sym("x");
    lval* y = lval_sym("y");
    lval* a = lval_num(25);
    lval* b = lval_num(30);

    q = lval_add(q, x);
    q = lval_add(q, y);

    v = lval_add(v, q);
    v = lval_add(v, a);
    v = lval_add(v, b);

    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* result = builtin_def(e, v);

    mu_assert(result->type == LVAL_SEXPR,
              "Defining a symbol should result in an empty sexpr");

    lval* val = lenv_get(e, lval_sym("x"));
    mu_assert(val->type == LVAL_NUM,
              "Getting x from environment should result in number");
    mu_assert(val->num == 25,
              "Getting x from environment should match 25");
    lval_delete(val);

    val = lenv_get(e, lval_sym("y"));
    mu_assert(val->type == LVAL_NUM,
              "Getting y from environment should result in number");
    mu_assert(val->num == 30,
              "Getting y from environment should match 30");
    lval_delete(val);
    lval_delete(result);

    lval* addition_expr = lval_sexpr();
    addition_expr = lval_add(addition_expr, lval_sym("+"));
    addition_expr = lval_add(addition_expr, lval_sym("x"));
    addition_expr = lval_add(addition_expr, lval_sym("y"));

    lval* addition_result = lval_eval(e, addition_expr);

    mu_assert(addition_result->type == LVAL_NUM,
              "Evaluating (+ x y) should result in a number");
    mu_assert(addition_result->num == 55,
              "Evaluating (+ x y) should result in an anser of 55");

    lval_delete(addition_result);
    lenv_delete(e);
}

MU_TEST(test_lval_lambda_success) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* val = lval_sexpr();
    lval* body = lval_qexpr();
    body = lval_add(body, lval_sym("+"));
    body = lval_add(body, lval_sym("x"));
    body = lval_add(body, lval_num(1));

    lval* args = lval_qexpr();
    args = lval_add(args, lval_sym("x"));

    val = lval_add(val, args);
    val = lval_add(val, body);

    lval* result = builtin_lambda(e, val);

    mu_assert(result->type == LVAL_FUN,
              "Creating a lambda should result in a function type.");
    lval_delete(result);
}

MU_TEST(test_lval_lambda_size) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* val = lval_sexpr();

    lval* args = lval_qexpr();
    args = lval_add(args, lval_sym("x"));

    val = lval_add(val, args);

    lval* result = builtin_lambda(e, val);

    mu_assert(result->type == LVAL_ERR,
              "Creating a lambda without a body should create an error");
    lval_delete(result);
}

MU_TEST(test_lval_lambda_optional) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* val = lval_sexpr();
    lval* body = lval_qexpr();
    body = lval_add(body, lval_sym("xs"));

    lval* args = lval_qexpr();
    args = lval_add(args, lval_sym("&"));
    args = lval_add(args, lval_sym("xs"));

    val = lval_add(val, args);
    val = lval_add(val, body);

    lval* result = builtin_lambda(e, val);

    lval* invoke = lval_sexpr();
    invoke = lval_add(invoke, result);
    invoke = lval_add(invoke, lval_num(1));
    invoke = lval_add(invoke, lval_num(100));

    lval* final_result = lval_eval(e, invoke);

    mu_assert(final_result->type == LVAL_QEXPR,
              "Optional arguments should be returned");
    mu_assert(final_result->cell[0]->num == 1,
              "Result should have 1 as the first element");
    mu_assert(final_result->cell[1]->num == 100,
              "Result should have 100 as the second element");

    lval_delete(final_result);
}

MU_TEST_SUITE(builtin_suite) {
    MU_RUN_TEST(test_lval_cons);
    MU_RUN_TEST(test_lval_list_success);
    MU_RUN_TEST(test_lval_list_qexpr);
    MU_RUN_TEST(test_lval_eval_success);
    MU_RUN_TEST(test_lval_eval_too_large);
    MU_RUN_TEST(test_lval_eval_non_qexpr);
    MU_RUN_TEST(test_lval_join_success);
    MU_RUN_TEST(test_lval_join_non_qexpr);
    MU_RUN_TEST(test_lval_head_success);
    MU_RUN_TEST(test_lval_head_too_many_arguments);
    MU_RUN_TEST(test_lval_head_empty_qexpr);
    MU_RUN_TEST(test_lval_tail_success);
    MU_RUN_TEST(test_lval_tail_too_many_arguments);
    MU_RUN_TEST(test_lval_tail_empty_qexpr);
    MU_RUN_TEST(test_lval_len_success);
    MU_RUN_TEST(test_lval_len_too_many);
    MU_RUN_TEST(test_lval_len_non_qexpr);
    MU_RUN_TEST(test_lval_init_success);
    MU_RUN_TEST(test_lval_init_too_many_arguments);
    MU_RUN_TEST(test_lval_init_non_qexpr);
    MU_RUN_TEST(test_lval_def_success);
    MU_RUN_TEST(test_lval_def_multiple_eval);
    MU_RUN_TEST(test_lval_lambda_success);
    MU_RUN_TEST(test_lval_lambda_size);
    MU_RUN_TEST(test_lval_lambda_optional);
}

MU_TEST(test_lval_copy_num) {
    lval* v = lval_num(15);
    lval* x = lval_copy(v);

    mu_assert(x->type == v->type,
              "Copy should produce same types");
    mu_assert(x->num == v->num,
              "Copy lval should match the original");
    lval_delete(x);
    lval_delete(v);
}

MU_TEST(test_lval_copy_err) {
    lval* v = lval_err("Testing error copying");
    lval* x = lval_copy(v);

    mu_assert(x->type == v->type,
              "Copy should produce same types");
    mu_assert(strcmp(x->err, v->err) == 0,
              "Copy lval should match the original");
    lval_delete(x);
    lval_delete(v);
}

MU_TEST(test_lval_copy_sym) {
    lval* v = lval_sym("sup");
    lval* x = lval_copy(v);

    mu_assert(x->type == v->type,
              "Copy should produce same types");
    mu_assert(strcmp(x->sym, v->sym) == 0,
              "Copy lval should match the original");
    lval_delete(x);
    lval_delete(v);
}
MU_TEST(test_lval_copy_sandqexpr) {
    lval* v = lval_qexpr();
    v = lval_add(v, lval_num(14));
    v = lval_add(v, lval_num(15));
    lval* x = lval_copy(v);

    mu_assert(x->type == v->type,
              "Copy should produce same types");
    for (int i = 0; i < v->count; i++) {
        mu_assert(x->cell[i]->num == v->cell[i]->num,
              "Copy lval should match the original");
    }
    lval_delete(x);
    lval_delete(v);
}

MU_TEST_SUITE(lval_copy_suite) {
    MU_RUN_TEST(test_lval_copy_num);
    MU_RUN_TEST(test_lval_copy_err);
    MU_RUN_TEST(test_lval_copy_sym);
    MU_RUN_TEST(test_lval_copy_sandqexpr);
}

int main() {
    MU_RUN_SUITE(builtin_suite);
    MU_RUN_SUITE(lval_copy_suite);
    MU_REPORT();
    MU_RETURN_VALUE();
}
