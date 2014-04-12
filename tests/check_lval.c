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

MU_TEST_SUITE(builtin_suite)
{
    MU_RUN_TEST(test_lval_cons);
    MU_RUN_TEST(test_lval_list_success);
    MU_RUN_TEST(test_lval_list_qexpr);
}

int main() {
    MU_RUN_SUITE(builtin_suite);
    MU_REPORT();
    MU_RETURN_VALUE();
}
