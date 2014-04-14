#include "../src/lenv.h"
#include "../src/lval.h"
#include "minunit/minunit.h"

MU_TEST(test_lenv_get_success) {
    lenv* e = lenv_new();
    lval* q = lval_qexpr();
    lval* k = lval_sym("nice");
    q = lval_add(q, lval_num(1));
    lenv_put(e, k, q);

    lval* result = lenv_get(e, k);

    mu_assert(result->type == q->type,
              "Fetched lval from environment doesn't match types.");
    mu_assert(result->cell[0]->type == LVAL_NUM,
              "Fetched lval from environment doesn't have the matching num");
    mu_assert(result->cell[0]->num == 1,
              "Fetched lval from environment doesn't have the matching num");
    lval_delete(q);
    lval_delete(k);
    lval_delete(result);
    lenv_delete(e);
}

MU_TEST(test_lenv_get_not_found) {
    lenv* e = lenv_new();
    lval* q = lval_qexpr();
    lval* k = lval_sym("nice");
    q = lval_add(q, lval_num(1));

    lval* result = lenv_get(e, k);

    mu_assert(result->type == LVAL_ERR,
              "Should get an error back for an empty lenv");
    lval_delete(q);
    lval_delete(k);
    lval_delete(result);
    lenv_delete(e);
}

MU_TEST(test_lenv_lookup_sym_success) {
    lenv* e = lenv_new();
    lval* k = lval_sym("sup");

    lval* v = lval_fun(builtin_add);
    lenv_put(e, k, v);

    lval* result = lenv_lookup_sym(e, v);

    mu_assert(result->type == LVAL_SYM,
              "Lookup failed to return a symbol");
    mu_assert(strcmp(result->sym, "sup") == 0,
              "Lockup's symbol doesn't match sup");
    lval_delete(result);
    lval_delete(v);
    lval_delete(k);
    lenv_delete(e);
}

MU_TEST_SUITE(lenv_add_remove_suite) {
    MU_RUN_TEST(test_lenv_get_success);
    MU_RUN_TEST(test_lenv_get_not_found);
    MU_RUN_TEST(test_lenv_lookup_sym_success);
}

int main() {
    MU_RUN_SUITE(lenv_add_remove_suite);
    MU_REPORT();
    MU_RETURN_VALUE();
}
