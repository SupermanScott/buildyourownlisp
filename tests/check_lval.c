#include "../src/lval.h"
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

    lval* result = builtin(v, "cons");

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

    lval* result = builtin(v, "list");
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

    lval* result = builtin(v, "list");
    mu_assert(result->type == LVAL_ERR,
              "When list function is handed a non-sexpr it should error out.");
    lval_delete(v);
}

MU_TEST(test_lval_eval_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    lval_add(q, lval_sym("+"));
    lval_add(q, lval_num(1));
    lval_add(q, lval_num(2));

    lval_add(v, q);

    lval* result = builtin(v, "eval");
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

    lval* result = builtin(v, "eval");
    mu_assert(result->type == LVAL_ERR,
              "When passed more then on Qexpr, eval should error out");
    lval_delete(result);
}

MU_TEST(test_lval_eval_non_qexpr) {
    lval* v = lval_sexpr();
    lval_add(v, lval_sym("+"));

    lval* result = builtin(v, "eval");
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

    lval* result = builtin(v, "join");
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

    lval* result = builtin(v, "join");
    mu_assert(result->type == LVAL_ERR,
              "Joining a qexpr to a number should error out");
    lval_delete(result);
}

MU_TEST(test_lval_head_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    q = lval_add(q, lval_num(10));
    v = lval_add(v, lval_add(q, lval_num(1)));

    lval* result = builtin(v, "head");
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

    lval* result = builtin(v, "head");
    mu_assert(result->type == LVAL_ERR,
              "Head takes only one Qexpr not many arguments.");
    lval_delete(result);
}

MU_TEST(test_lval_head_empty_qexpr) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    v = lval_add(v, q);

    lval* result = builtin(v, "head");
    mu_assert(result->type == LVAL_ERR,
              "Head requires a non empty qexpr.");
    lval_delete(result);
}

MU_TEST(test_lval_tail_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    q = lval_add(q, lval_num(10));
    v = lval_add(v, lval_add(q, lval_num(1)));

    lval* result = builtin(v, "tail");
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

    lval* result = builtin(v, "tail");
    mu_assert(result->type == LVAL_ERR,
              "Tail takes only one Qexpr not many arguments.");
    lval_delete(result);
}

MU_TEST(test_lval_tail_empty_qexpr) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    v = lval_add(v, q);

    lval* result = builtin(v, "tail");
    mu_assert(result->type == LVAL_ERR,
              "Tail requires a non empty qexpr.");
    lval_delete(result);
}

MU_TEST(test_lval_len_success) {
    lval* v = lval_sexpr();
    lval* q = lval_qexpr();
    v = lval_add(v, lval_add(lval_add(q, lval_num(200)), lval_num(600)));

    lval* result = builtin(v, "len");

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

    lval* result = builtin(v, "len");

    mu_assert(result->type == LVAL_ERR,
              "Len called with two arguments should error out.");
    lval_delete(result);
}

MU_TEST(test_lval_len_non_qexpr) {
    lval* v = lval_sexpr();
    lval* q = lval_num(1);
    v = lval_add(v, q);

    lval* result = builtin(v, "len");

    mu_assert(result->type == LVAL_ERR,
              "Len when called without a qexpr should error out");
    lval_delete(result);
}

MU_TEST_SUITE(builtin_suite)
{
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
}

int main() {
    MU_RUN_SUITE(builtin_suite);
    MU_REPORT();
    MU_RETURN_VALUE();
}
