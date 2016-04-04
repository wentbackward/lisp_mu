#include "lisp_mu.h"
#include <assert.h>

// Testing helpers
int test_ctr = 0;
#define assert_ctr(_exp)   test_ctr++; assert(_exp)

/*
void test_sum_int();
void test_sum_double();
void test_lisp_list();
*/

void test_lisp_sizeof();
void test_lisp_fixnum();
void test_lisp_symbol();
void test_basic_nil();
void test_nil_list();
void test_rest_nil();
void test_lisp_list();
void test_lisp_strings();
void test_lisp_quote();
void test_lisp_float();
void test_gc_collect_all();
void test_cons();
void test_equals();
void test_def_undef();
void test_eval();

/**
 * The following tests are for parsing lisp like structures into
 * an internal representation.
 */
void parse_tests() {
    test_lisp_sizeof();
    test_lisp_fixnum();
    test_lisp_symbol();
    test_basic_nil();
    test_rest_nil();
    test_nil_list();
    test_lisp_list();
    test_lisp_strings();
    test_lisp_quote();
    test_lisp_float();
}

int main() {
    const int iters = 500;
    for(int i=0; i< iters ; ++i) {
        // All parsing scenarios
        parse_tests();

        test_equals();

        // garbage collect all memory
        test_gc_collect_all();

        // Ensure the sanity of cons'ing
        test_cons();

        // Eval / Apply
        test_eval();

        // lambda

        // begin

        // Def, Undef
//        test_def_undef();

        // test_sum_int();

        // test_sum_double();

    }

    printf("Successful Tests: %d\n", test_ctr);
    return 0;
}

void test_eval() {
    lisp_init();
    cell exp, result;

    // Test for self evaluating expressions

    result = eval( exp = mkfixnum(3), nil);
    assert_ctr(exp == result && "test_eval() :: fixnums should be returned");
    result = eval( exp = mkstring("Test"), nil);
    assert_ctr(exp == result && "test_eval() :: strings should be returned");
#ifdef WITH_FLOATING_POINT
    result = eval( exp = mkfloat(1.25), nil);
    assert_ctr(exp == result && "test_eval() :: floats should be returned");
#endif

    // Test for quoted expressions
    exp = mkstring("Test");
    result = eval( quote(exp), nil );
    assert_ctr(lisp_equals(exp, result) && "test_eval() :: quote should return the expression untouched");

    exp = mksym("A");
    result = eval( quote(exp), nil );
    assert_ctr(lisp_equals(exp, result) && "test_eval() :: quote should return the expression untouched");

    exp = mkfixnum(345);
    result = eval( quote(exp), nil );
    assert_ctr(lisp_equals(exp, result) && "test_eval() :: quote should return the expression untouched");

    lisp_cleanup();
}

void test_def_undef() {
    lisp_init();


    lisp_cleanup();
}



void test_equals() {
    lisp_init();
    cell exp1, exp2;

    exp1 = nil;
    exp2 = nil;
    assert_ctr(lisp_equals(exp1, exp2) && "test_equals() :: nil is always nil");

    exp1 = cons(nil, nil);
    exp2 = cons(nil, nil);
    assert_ctr(!lisp_equals(exp1, exp2) && "test_equals() :: not the same location");
    exp2 = exp1;
    assert_ctr(lisp_equals(exp1, exp2) && "test_equals() :: it is the same location");


    exp1 = mksym("A");
    exp2 = mksym("A");
    assert_ctr(lisp_equals(exp1, exp2) && "test_equals() :: like symbols are equivalent");

    exp2 = mksym("B");
    assert_ctr(!lisp_equals(exp1, exp2) && "test_equals() :: unlike symbols are not equivalent");

    exp1 = mkstring("ABC");
    exp2 = mkstring("ABC");
    assert_ctr(lisp_equals(exp1, exp2) && "test_equals() :: like strings are equivalent");

    exp2 = mkstring("CBA");
    assert_ctr(!lisp_equals(exp1, exp2) && "test_equals() :: unlike strings are not equivalent");

    exp1 = mkerror("ABC");
    exp2 = mkerror("ABC");
    assert_ctr(lisp_equals(exp1, exp2) && "test_equals() :: like errors are equivalent");

    exp2 = mkerror("CBA");
    assert_ctr(!lisp_equals(exp1, exp2) && "test_equals() :: unlike errors are not equivalent");

    exp1 = mkfixnum(45);
    exp2 = mkfixnum(45);
    assert_ctr(lisp_equals(exp1, exp2) && "test_equals() :: like numbers are equivalent");

    exp2 = mkfixnum(42);
    assert_ctr(!lisp_equals(exp1, exp2) && "test_equals() :: unlike numbers are not equivalent");

#ifdef WITH_FLOATING_POINT
    exp1 = mkfloat(450.5);
    exp2 = mkfloat(450.5);
    assert_ctr(lisp_equals(exp1, exp2) && "test_equals() :: like numbers are equivalent");

    exp2 = mkfloat(42.5);
    assert_ctr(!lisp_equals(exp1, exp2) && "test_equals() :: unlike numbers are not equivalent");
#endif

    lisp_cleanup();
}

void test_cons() {
    lisp_init();

    cell exp, n1, n2;
    n1 = mkfixnum(3);
    n2 = mkfixnum(6);
    exp = cons(n1, n2);

    assert_ctr(exp -> type == CONS && "test_cons() :: should be a cons");
    assert_ctr(fixnum(car(exp)) == 3  && "test_cons() :: Symbol should be 3");
    assert_ctr(fixnum(cdr(exp)) == 6  && "test_cons() :: Symbol should be 6");

    lisp_cleanup();
}

void test_gc_collect_all() {
    cell exp;
    cell obj;
    const char *ptr;
    lisp_init();
#ifdef DEBUG
    lisp_pprint(all_objects,0);
#endif

    int l = lisp_length(all_objects);

    assert_ctr(lisp_length(all_objects) > 0 && "test_gc_collect_all() :: There is a single nil object plus all inits");
    ptr = "(1 2 3)";
    exp = lisp_read(&ptr);

#ifdef DEBUG
    lisp_print_list(all_objects);
    lisp_pprint(all_objects,0);
#endif
    assert_ctr((lisp_length(all_objects) == 6 + l) && "test_gc_collect_all() :: 6 for the list");

    obj = find_object(car(exp), all_objects);
    assert_ctr(car(exp) == car(obj) && "test_gc_collect_all() :: can find an object in all_objects");

    obj = find_object(second(exp), all_objects);
    assert_ctr(second(exp) == car(obj) && "test_gc_collect_all() :: can find another object in all_objects");

    obj = find_object(rest(exp), all_objects);
    assert_ctr(rest(exp) == car(obj) && "test_gc_collect_all() :: can find a cons in all_objects");

    lisp_free(true);
    assert_ctr(lisp_length(all_objects) == 1 && "test_gc_collect_all() :: Everything gone, except the final nil");

    lisp_cleanup();
}

void test_lisp_float() {
#ifdef WITH_FLOATING_POINT
    cell exp;
    const char *ptr;
    lisp_init();

    ptr = "3.3";
    exp = lisp_read(&ptr);
    assert_ctr(exp -> type == FLOAT && "test_lisp_floater() :: returns a FLOAT object");
    assert_ctr(floater(exp) == 3.3 && "test_lisp_floater() :: returns value 3.3");

    lisp_cleanup();

#endif // WITH_DOUBLE

}

void test_lisp_quote() {
    lisp_init();
    cell exp;
    const char *ptr;

    ptr = "'A";
    exp = lisp_read(&ptr);
    assert_ctr(car(exp) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(car(exp), "QUOTE") && "test_lisp_quote() :: Symbol should be a quote");
    assert_ctr(cadr(exp) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(cadr(exp),"A") && "test_lisp_quote() :: Parsing should not be finished");

    lisp_cleanup();
}

void test_lisp_strings() {
    cell exp;
    const char *ptr;
    lisp_init();

    ptr = "\"a string\"";
    exp = lisp_read(&ptr);
    assert_ctr(exp -> type == STRING && "test_lisp_string() :: returns a STRING object");
    assert_ctr(lisp_eq(exp, "a string") && "test_lisp_string() :: string should have spaces");

    ptr = "\"a (string)\"";
    exp = lisp_read(&ptr);
    assert_ctr(lisp_eq(exp, "a (string)") && "test_lisp_string() :: string should have brackets");

    ptr = "\"a \\\"str\\\\ing\\\"";
    exp = lisp_read(&ptr);
    assert_ctr(lisp_eq(exp, "a \"str\\ing\"") && "test_lisp_string() :: string should have escaped chars");

    lisp_cleanup();
}

void test_lisp_sizeof() {
    assert_ctr((lisp_sizeof(SYM)    == sizeof(char)  && "lisp_sizeof returns underlying system sizes"));
    assert_ctr((lisp_sizeof(STRING) == sizeof(char)  && "lisp_sizeof returns underlying system sizes"));
    assert_ctr((lisp_sizeof(FIXNUM) == sizeof(long)  && "lisp_sizeof returns underlying system sizes"));
    assert_ctr((lisp_sizeof(CONS)   == sizeof(lisp_cell) && "lisp_sizeof returns underlying system sizes"));
}

void test_lisp_symbol() {
    cell exp1;
    cell exp2;
    const char *ptr, *ptr2;

    lisp_init();

    ptr = "A";
    exp1 = lisp_read(&ptr);
    assert_ctr(exp1 -> type == SYM && "test_lisp_symbol() :: returns a SYM object");
    assert_ctr(lisp_eq(exp1, (any) "A") && "test_lisp_symbol() :: returns value A");

    ptr = "ABCD";
    exp1 = lisp_read(&ptr);
    assert_ctr(exp1 -> type == SYM && "test_lisp_symbol() :: returns a SYM object");
    assert_ctr(lisp_eq(exp1, (any) "ABCD") && "test_lisp_symbol() :: multiple chars returns value ABCD");

    ptr = "XYZ"; ptr2 = "XYZ";
    exp1 = lisp_read(&ptr);
    exp2 = lisp_read(&ptr2);
    assert_ctr(lisp_equals(exp1, exp2) && "test_lisp_symbol() :: two symbols with same name are identical");

    ptr = "XYZ ABC"; ptr2 = "XYZ DEF";
    exp1 = lisp_read(&ptr);
    exp2 = lisp_read(&ptr2);
    assert_ctr(lisp_equals(exp1, exp2) && "test_lisp_symbol() :: Only first symbol is parsed");

    lisp_cleanup();
}

void test_lisp_fixnum() {
    cell exp;
    const char *ptr;
    lisp_init();

    ptr = "3";
    exp = lisp_read(&ptr);
    assert_ctr(exp -> type == FIXNUM && "test_lisp_fixnum() :: returns a FIXNUM object");
    assert_ctr(fixnum(exp) == 3 && "test_lisp_fixnum() :: returns value 3");

    ptr = "  4  ";
    exp = lisp_read(&ptr);
    assert_ctr(exp -> type == FIXNUM && "test_lisp_fixnum() :: with padded spaces returns a FIXNUM object");
    assert_ctr(fixnum(exp) == 4 && "test_lisp_fixnum() :: with padded spaces returns value 4");

    ptr = "5432";
    exp = lisp_read(&ptr);
    assert_ctr(exp -> type == FIXNUM && "test_lisp_fixnum() :: several digits returns a FIXNUM object");
    assert_ctr(fixnum(exp) == 5432 && "test_lisp_fixnum() :: several digits returns value 4");

    lisp_cleanup();
}

void test_basic_nil() {
    cell exp;
    const char *ptr;

    lisp_init();

    ptr = "";
    exp = lisp_read(&ptr);
    assert_ctr(lisp_equals(exp, nil) && "test_basic_nil() :: All NIL objects should be equal");

    ptr = "   \n\n\n\t     ";
    exp = lisp_read(&ptr);
    assert_ctr(lisp_equals(exp, nil) && "test_basic_nil() :: All NIL objects should be equal");

    lisp_cleanup();
}

void test_rest_nil() {
    lisp_init();
    cell exp = mksym("TEST");
    assert_ctr(nullp(exp->rest) && "test_rest_nil() :: The last element of a list should always be nil");
    lisp_cleanup();
}

void test_nil_list() {
    cell exp;
    const char *ptr;

    lisp_init();

    ptr="()";
    exp = lisp_read(&ptr);
    assert_ctr(lisp_equals(exp, nil) && "test_nil_list() :: () is the nil object");

    ptr="  (   \n  )  ";
    exp = lisp_read(&ptr);
    assert_ctr(lisp_equals(exp, nil) && "test_nil_list() :: (), even full of whitespace, is still the nil object");

    lisp_cleanup();
}

void test_lisp_list() {
    cell exp1;
    cell exp2;

    const char *ptr;

    lisp_init();

    ptr = "(1 2 3)";
    exp1 = lisp_read(&ptr);
    assert_ctr(listp(exp1) && "test_lisp_list :: Read a basic list");
    assert_ctr(lisp_length(exp1) == 3 && "test_lisp_list :: Basic list length should be 3");

    assert_ctr(car(exp1)->type == FIXNUM && "test_lisp_list :: Element 1 should be a fixnum");
    assert_ctr(fixnum(first(exp1)) == 1 && "test_lisp_list :: Element 1 should be 1");

    assert_ctr(second(exp1)->type == FIXNUM && "test_lisp_list :: Element 2 should be a fixnum");
    assert_ctr(fixnum(second(exp1)) == 2 && "test_lisp_list :: Element 2 should be 2");

    assert_ctr(third(exp1)->type == FIXNUM && "test_lisp_list :: Element 3 should be a fixnum");
    assert_ctr(fixnum(third(exp1)) == 3 && "test_lisp_list :: Element 3 should be 3");

    assert_ctr(nth(exp1,2)->type == FIXNUM && "test_lisp_list :: Test nth 2 returns 2nd element");
    assert_ctr(fixnum(nth(exp1,2)) == 2 && "test_lisp_list :: Test nth 2 returns 2nd element");

//    lisp_pprint(exp1,0);

    ptr = "(100 200 (300 400 500 (AAA XYZ C) 600) 700)";
    exp1 = lisp_read(&ptr);
    assert_ctr(lisp_length(exp1) == 4 && "test_lisp_list :: List with nested length should be 5");
    assert_ctr(fixnum(first(exp1)) == 100 && "test_lisp_list :: Nested list first is 100");
    assert_ctr(fixnum(first(rest(exp1))) == 200 && "test_lisp_list :: Nested list first is 200");

//    lisp_pprint(exp1,0);

    lisp_cleanup();
}

/*


void test_sum_double() {
#ifdef WITH_FLOATING_POINT
    assert_ctr( (sum_double(2, 15.5, 56.6) == 72.1) && "test_sum_double() :: sums two doubles" );
    ++tests_run;
#endif
}

void test_sum_int() {
    assert_ctr((sum_int(2, 15, 56) == 71) && "test_sum_int() :: sums two ints" );
    ++tests_run;
}
*/