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

MU_TEST(test_lval_func_success) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* val = lval_sexpr();
    lval* args = lval_qexpr();
    args = lval_add(args, lval_sym("add-together"));
    args = lval_add(args, lval_sym("x"));
    args = lval_add(args, lval_sym("y"));

    lval* body = lval_qexpr();
    body = lval_add(body, lval_sym("+"));
    body = lval_add(body, lval_sym("x"));
    body = lval_add(body, lval_sym("y"));

    val = lval_add(val, args);
    val = lval_add(val, body);

    lval* result = builtin_fun(e, val);

    mu_assert(result->type == LVAL_FUN,
              "Defining a func should result in a fun coming back");
    lval* stored = lenv_get(e, lval_sym("add-together"));
    mu_assert(stored->type == LVAL_FUN,
              "Getting add-together from env should result in a fun");
}

MU_TEST(test_lval_gt) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* comp = lval_sexpr();
    comp = lval_add(comp, lval_num(1));
    comp = lval_add(comp, lval_num(2));

    lval* result = builtin_gt(e, comp);
    mu_assert(result->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result->num == 0,
              "1 > 2 should return False");
    lval_delete(result);

    lval* comp_lt = lval_sexpr();
    comp = lval_add(comp_lt, lval_num(2));
    comp = lval_add(comp_lt, lval_num(1));

    lval* result_lt = builtin_gt(e, comp);
    mu_assert(result_lt->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result_lt->num > 0,
              "2 > 1 should return True");
    lval_delete(result_lt);

    lval* comp_eq = lval_sexpr();
    comp = lval_add(comp_eq, lval_num(2));
    comp = lval_add(comp_eq, lval_num(2));

    lval* result_eq = builtin_gt(e, comp);
    mu_assert(result_eq->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result_eq->num == 0,
              "2 > 2 should return False");
    lval_delete(result_eq);

}

MU_TEST(test_lval_lt) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* comp = lval_sexpr();
    comp = lval_add(comp, lval_num(1));
    comp = lval_add(comp, lval_num(2));

    lval* result = builtin_lt(e, comp);
    mu_assert(result->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result->num > 0,
              "1 < 2 should return True");
    lval_delete(result);

    lval* comp_lt = lval_sexpr();
    comp = lval_add(comp_lt, lval_num(2));
    comp = lval_add(comp_lt, lval_num(1));

    lval* result_lt = builtin_lt(e, comp);
    mu_assert(result_lt->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result_lt->num == 0,
              "2 < 1 should return False");
    lval_delete(result_lt);

    lval* comp_eq = lval_sexpr();
    comp = lval_add(comp_eq, lval_num(2));
    comp = lval_add(comp_eq, lval_num(2));

    lval* result_eq = builtin_lt(e, comp);
    mu_assert(result_eq->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result_eq->num == 0,
              "2 < 2 should return False");
    lval_delete(result_eq);
}

MU_TEST(test_lval_lte) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* comp = lval_sexpr();
    comp = lval_add(comp, lval_num(1));
    comp = lval_add(comp, lval_num(2));

    lval* result = builtin_le(e, comp);
    mu_assert(result->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result->num > 0,
              "1 <= 2 should return True");
    lval_delete(result);

    lval* comp_lt = lval_sexpr();
    comp = lval_add(comp_lt, lval_num(2));
    comp = lval_add(comp_lt, lval_num(1));

    lval* result_lt = builtin_le(e, comp);
    mu_assert(result_lt->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result_lt->num == 0,
              "2 <= 1 should return False");
    lval_delete(result_lt);

    lval* comp_eq = lval_sexpr();
    comp = lval_add(comp_eq, lval_num(2));
    comp = lval_add(comp_eq, lval_num(2));

    lval* result_eq = builtin_le(e, comp);
    mu_assert(result_eq->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result_eq->num > 0,
              "2 <= 2 should return True");
    lval_delete(result_eq);
}

MU_TEST(test_lval_gte) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* comp = lval_sexpr();
    comp = lval_add(comp, lval_num(1));
    comp = lval_add(comp, lval_num(2));

    lval* result = builtin_ge(e, comp);
    mu_assert(result->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result->num == 0,
              "1 >= 2 should return False");
    lval_delete(result);

    lval* comp_lt = lval_sexpr();
    comp = lval_add(comp_lt, lval_num(2));
    comp = lval_add(comp_lt, lval_num(1));

    lval* result_lt = builtin_ge(e, comp);
    mu_assert(result_lt->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result_lt->num > 0,
              "2 >= 1 should return True");
    lval_delete(result_lt);

    lval* comp_eq = lval_sexpr();
    comp = lval_add(comp_eq, lval_num(2));
    comp = lval_add(comp_eq, lval_num(2));

    lval* result_eq = builtin_ge(e, comp);
    mu_assert(result_eq->type == LVAL_NUM,
              "Comparing two numbers should return a LVAL_NUM");
    mu_assert(result_eq->num > 0,
              "2 >= 2 should return True");
    lval_delete(result_eq);
}

MU_TEST(test_lval_eq_success) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* not_same_type = builtin_eq(e,
                                     lval_add(lval_add(lval_sexpr(), lval_num(1)),
                                              lval_sym("sup")));
    mu_assert(not_same_type->type == LVAL_NUM,
              "When comparing 1 and symbol 'sup', eq should return a number");
    mu_assert(not_same_type->num == 0,
              "Comparing 1 and symbol 'sup' should result in False");
    lval_delete(not_same_type);

    lval* num_check = builtin_eq(e,
                                 lval_add(lval_add(lval_sexpr(), lval_num(1)),
                                          lval_num(1)));
    mu_assert(num_check->type == LVAL_NUM,
              "When comparing 1 and 1, eq should return a number");
    mu_assert(num_check->num == 1,
              "Comparing 1 and 1 should result in True");
    lval_delete(num_check);

    lval* err_check = builtin_eq(e,
                                 lval_add(lval_add(lval_sexpr(), lval_err("sup")),
                                          lval_err("sup")));
    mu_assert(err_check->type == LVAL_NUM,
              "When comparing two errors, eq should return a LVAL_NUM");
    mu_assert(err_check->num == 1,
              "Comparing two errors of 'sup' should return True");
    lval_delete(err_check);

    lval* sym_check = builtin_eq(e,
                                 lval_add(lval_add(lval_sexpr(), lval_sym("sup")),
                                          lval_sym("sup")));
    mu_assert(sym_check->type == LVAL_NUM,
              "When comparing two syms, eq should return a LVAL_NUM");
    mu_assert(sym_check->num == 1,
              "Comparing two syms of 'sup' should return True");
    lval_delete(sym_check);

    lval* builtin_check = builtin_eq(e,
                                 lval_add(lval_add(lval_sexpr(),
                                                   lenv_get(e, lval_sym("+"))),
                                          lenv_get(e, lval_sym("+"))));
    mu_assert(builtin_check->type == LVAL_NUM,
              "When comparing two copies of builtin_add, eq should return a"\
              " LVAL_NUM");
    mu_assert(builtin_check->num == 1,
              "Comparing two copies of builtin_add, eq should return True");
    lval_delete(builtin_check);

    // Comparing lval_lambda's also test the logic for qexpr. Which also
    // happens to be the same logic for sexpr. So three birds, one stone.
    lval* args = lval_qexpr();
    args = lval_add(args, lval_sym("add-together"));
    args = lval_add(args, lval_sym("x"));
    args = lval_add(args, lval_sym("y"));

    lval* body = lval_qexpr();
    body = lval_add(body, lval_sym("+"));
    body = lval_add(body, lval_sym("x"));
    body = lval_add(body, lval_sym("y"));
    lval* lambda_check = builtin_eq(e,
                                 lval_add(lval_add(lval_sexpr(),
                                                   lval_lambda(lval_copy(args),
                                                               lval_copy(body))),
                                          lval_lambda(args, body)));
    mu_assert(lambda_check->type == LVAL_NUM,
              "When comparing two equivalent lambdas, eq should return a"\
              " LVAL_NUM");
    mu_assert(lambda_check->num == 1,
              "Comparing two equivalent lambdas eq should return True");
    lval_delete(lambda_check);
}

MU_TEST(test_lval_ne_success) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);
    lval* num_check = builtin_ne(e,
                                 lval_add(lval_add(lval_sexpr(), lval_num(1)),
                                          lval_num(1)));
    mu_assert(num_check->type == LVAL_NUM,
              "When comparing 1 and 1, eq should return a number");
    mu_assert(num_check->num == 0,
              "Comparing 1 and 1 ne should result in False");
    lval_delete(num_check);
}

MU_TEST(test_lval_if_success) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* a = lval_sexpr();
    a = lval_add(a, lval_num(1));
    a = lval_add(a, lval_add(lval_qexpr(), lval_num(100)));
    a = lval_add(a, lval_add(lval_qexpr(), lval_num(200)));
    lval* true_result = builtin_if(e, a);
    mu_assert(true_result->type == LVAL_NUM,
              "(if 1 {100} {200}) should result in a LVAL_NUM");
    mu_assert(true_result->num == 100,
              "(if 1 {100} {200}) should result in 100");
    lval_delete(true_result);

    lval* args = lval_sexpr();
    args = lval_add(args, lval_num(0));
    args = lval_add(args, lval_add(lval_qexpr(), lval_num(100)));
    args = lval_add(args, lval_add(lval_qexpr(), lval_num(200)));
    lval* false_result = builtin_if(e, args);
    mu_assert(false_result->type == LVAL_NUM,
              "(if 0 {100} {200}) should result in a LVAL_NUM");
    mu_assert(false_result->num == 200,
              "(if 0 {100} {200}) should result in 200");
    lval_delete(false_result);
}

MU_TEST(test_lval_or_success) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* condition = lval_num(1);
    lval* t = lval_sexpr();
    t = lval_add(t, condition);

    lval* result = builtin_or(e, t);

    mu_assert(result->type == LVAL_NUM,
              "(or 1) should return an LVAL_NUM");
    mu_assert(result->num == 1,
              "(or 1) should return 1");
    lval_delete(result);
}

MU_TEST(test_lval_or_fail) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* condition = lval_num(0);
    lval* t = lval_sexpr();
    t = lval_add(t, condition);

    lval* result = builtin_or(e, t);

    mu_assert(result->type == LVAL_NUM,
              "(or 0) should return an LVAL_NUM");
    mu_assert(result->num == 0,
              "(or 0) should return 0");
    lval_delete(result);
}

MU_TEST(test_lval_or_fail_then_success) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* t = lval_sexpr();
    t = lval_add(t, lval_num(0));
    t = lval_add(t, lval_num(1));

    lval* result = builtin_or(e, t);

    mu_assert(result->type == LVAL_NUM,
              "(or 0 1) should return an LVAL_NUM");
    mu_assert(result->num == 1,
              "(or 0 1) should return 1");
    lval_delete(result);
}

MU_TEST(test_lval_and_success) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* condition = lval_num(1);
    lval* t = lval_sexpr();
    t = lval_add(t, condition);

    lval* result = builtin_and(e, t);

    mu_assert(result->type == LVAL_NUM,
              "(and 1) should return an LVAL_NUM");
    mu_assert(result->num == 1,
              "(and 1) should return 1");
    lval_delete(result);
}

MU_TEST(test_lval_and_success_multiple) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* t = lval_sexpr();
    t = lval_add(t, lval_num(1));
    t = lval_add(t, lval_num(1));

    lval* result = builtin_and(e, t);

    mu_assert(result->type == LVAL_NUM,
              "(and 1 1) should return an LVAL_NUM");
    mu_assert(result->num == 1,
              "(and 1 1) should return 1");
    lval_delete(result);
}

MU_TEST(test_lval_and_fail_multiple) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* t = lval_sexpr();
    t = lval_add(t, lval_num(1));
    t = lval_add(t, lval_num(0));

    lval* result = builtin_and(e, t);
    mu_assert(result->type == LVAL_NUM,
              "(and 1 0) should return an LVAL_NUM");
    mu_assert(result->num == 0,
              "(and 1 0) should return 0");
    lval_delete(result);
}

MU_TEST(test_lval_not_success) {
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval* t = lval_sexpr();
    t = lval_add(t, lval_num(0));

    lval* result = builtin_not(e, t);
    mu_assert(result->type == LVAL_NUM,
              "Calling (not 0) should result in a LVAL_NUM");
    mu_assert(result->num == 1,
              "Calling (not 0) should result in 1");

    lval_delete(result);

    lval* failure_result = builtin_not(e, lval_add(lval_sexpr(), lval_num(1)));
    mu_assert(failure_result->type == LVAL_NUM,
              "Calling (not 1) should result in a LVAL_NUM");
    mu_assert(failure_result->num == 0,
              "Calling (not 1) should result in 0");
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
    MU_RUN_TEST(test_lval_func_success);
    MU_RUN_TEST(test_lval_gt);
    MU_RUN_TEST(test_lval_lt);
    MU_RUN_TEST(test_lval_lte);
    MU_RUN_TEST(test_lval_gte);
    MU_RUN_TEST(test_lval_eq_success);
    MU_RUN_TEST(test_lval_ne_success);
    MU_RUN_TEST(test_lval_if_success);
    MU_RUN_TEST(test_lval_or_success);
    MU_RUN_TEST(test_lval_or_fail);
    MU_RUN_TEST(test_lval_or_fail_then_success);
    MU_RUN_TEST(test_lval_and_success);
    MU_RUN_TEST(test_lval_and_success_multiple);
    MU_RUN_TEST(test_lval_and_fail_multiple);
    MU_RUN_TEST(test_lval_not_success);
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
