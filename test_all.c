#include "lisp_mu.h"
#include <stdio.h>
#include "tinyprintf.h"
#include <assert.h>
#include <time.h>

#define STR(...) #__VA_ARGS__

// Testing helpers
int test_ctr = 0;
#define assert_ctr(_exp)    test_ctr++; assert(_exp)
#define start_timer(S)      S = clock()
#define stop_timer(E)       E = clock()
#define time_diff(S, E)     ((int)((E - S) * 1000 / CLOCKS_PER_SEC))


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
void test_mklist();
void test_lisp_strings();
void test_lisp_quote();
void test_lisp_float();
void test_gc_collect_all();
void test_cons();
void test_equals();
void test_eval_define();
void test_eval();
void test_eval_quoted();
void test_eval_variablep();
void test_eval_assignment();
void test_eval_if();
void test_eval_lambda();
void test_eval_begin();
void test_eval_cond();
void test_eval_apply();
void test_map_reduce();
void test_make_functions();
void test_eval_primitive();
void test_eval_environment();
void test_read_eval();
void test_read_eval2();
void test_read_eval3();

void print_global_env();


void my_putc( void* p, char c) {
    putc(c, stdout);
    fflush(stdout);
}

int main() {
    init_printf(NULL, my_putc);

    clock_t t_start, t_end;
    start_timer(t_start);
    const int iters = 10000;
    for(int i=0; i< iters ; ++i) {

        // General utilities
        test_lisp_sizeof();             // Provides default sizes based on the LISP type
        test_lisp_fixnum();             // Integers in lisp
        test_lisp_symbol();             // Handling of symbols
        test_basic_nil();               // Handling of nil objects
        test_rest_nil();                // Handling of REST when confronted with nil
        test_nil_list();                // Empty lists are nil
        test_lisp_list();               // Consing into lists
        test_mklist();                  // Function to simplify cons'ing lists together
        test_lisp_strings();            // Processing string types
        test_lisp_quote();              // Quoting of lisp expressions
        test_lisp_float();              // Floating point, if enabled in mu_lisp.h
        test_equals();                  // Equivalence of lisp objects and between lisp / c objects
        test_gc_collect_all();          // garbage collect all memory (i.e. complete cleanup)

        // Actual LISP functionality
        test_cons();                    // Ensure the sanity of cons'ing
        test_eval();                    // Test basic dispatch within evaluator
        test_eval_quoted();             // eval quoted
        test_eval_define();             // eval definition
        test_eval_assignment();         // eval assignment
        test_eval_variablep();          // Evaluation of a variable (symbol lookup)
        test_eval_if();                 // eval if
        test_eval_lambda();             // eval lambda
        test_eval_begin();              // eval begin
        test_make_functions();          // Making and calling primitive functions
        test_map_reduce();              // mapper and reducer
        test_eval_apply();              // eval application
        test_eval_primitive();          // eval then apply primitives
        test_eval_environment();

        test_read_eval();               // operators +,-,*,/ and defining functions using them
        test_read_eval2();              // =, multiple defines and recursion
        test_read_eval3();              // Even more primitives

        continue;
        test_eval_cond();               // TODO: eval cond
        // TODO: Floats

        // Libraries exposed from C
        // TODO: Maths library
        // TODO: MAXLEN bounds tests
        // TODO: Garbace collect non referenced objects

    }
    stop_timer(t_end);

    printf("Test took: %d.%ds\n", time_diff(t_start, t_end)/1000, time_diff(t_start, t_end) % 1000);
    printf("Successful Tests: %d\n", test_ctr);
    if (time_diff(t_start, t_end) > 0) {
        printf("Tests per ms: %d\n", test_ctr / time_diff(t_start, t_end));
    } else {
        printf("Could not measure the speed, too fast\n");
    }
    return 0;
}


void test_read_eval2() {
    lisp_init();
    cell exp, result;
    const char * prog;

    prog = "(= 1 1)";
    exp = lisp_read(&prog);
    result = eval(exp, global_env);
    assert_ctr((!nullp(result)) && "Can check for equality of numbers");

    prog = "(= 1 2)";
    exp = lisp_read(&prog);
    result = eval(exp, global_env);
    assert_ctr((nullp(result)) && "Can check for inequality of numbers");

    prog = "(= 'x 'x)";
    exp = lisp_read(&prog);
    result = eval(exp, global_env);
    assert_ctr((!nullp(result)) && "Can check for equality of symbols");

    prog = "(= 'x 'y)";
    exp = lisp_read(&prog);
    result = eval(exp, global_env);
    assert_ctr((nullp(result)) && "Can check for inequality of symbols");

    prog = STR(
            (begin
                    (define (square x)
                       (* x x))
                    (define (sum-of-squares x y)
                       (+ (square x) (square y)))
                    (define (f a)
                       (sum-of-squares (+ a 1) (+ a 2)))
                    (f 5))
    );
    exp = lisp_read(&prog);
    result = eval(exp, global_env);
    assert_ctr(fixnum(result) == 85 && "Multiple definitions");

    //                         (print "f " n ", ")
    prog = STR(
            (begin
                    (define (factorial n)
                        (if (= n 1)
                            1
                            (* n (factorial (- n 1)))))
                    (factorial 5))
    );
    exp = lisp_read(&prog);
    result = eval(exp, global_env);
    assert_ctr(fixnum(result) == 120 && "Handles recursion");

    lisp_cleanup();
}

void test_read_eval3() {
    lisp_init();
    cell exp, result;
    const char * prog;

    // TODO: Leaving this until I have a strategy on floats

//    prog = STR(
//            (define (sqrt x)
//                (define (good-enough? guess)
//                   (< (abs (- (square guess) x)) 0.001))
//                (define (improve guess)
//                    (average guess (/ x guess)))
//                (define (sqrt-iter guess)
//                    (if (good-enough? guess)
//                        guess
//                        (sqrt-iter (improve guess))))
//                (sqrt-iter 1.0))
//            (sqrt 18)
//    );
//    exp = lisp_read(&prog);
//    result = eval(exp, global_env);
//    assert_ctr(fixnum(result) == 4 && "sqrt with nested defines");

    lisp_cleanup();
}

void test_read_eval() {
    lisp_init();
    cell exp, result;
    const char * prog;

    prog = "1";
    result = eval(lisp_read(&prog), global_env);
    assert_ctr(fixnum(result) == 1 && "We can read a raw integer");

    prog = "'K";
    result = eval(lisp_read(&prog), global_env);
    assert_ctr(result->type == SYM && lisp_eq(result, "K") && "We can read a symbol");

    prog = "(+ 1 2 3)";
    result = eval(lisp_read(&prog), global_env);
    assert_ctr(fixnum(result) == 6 && "We can execute a primitive");

    prog = "(* 1 2 3)";
    result = eval(lisp_read(&prog), global_env);
    assert_ctr(fixnum(result) == 6 && "We can execute a primitive");

    prog = "(begin (define x 100) x)";
    result = eval(lisp_read(&prog), global_env);
    assert_ctr(fixnum(result) == 100 && "We can lookup a variable value");

    prog = "x";
    result = eval(lisp_read(&prog), global_env);
    assert_ctr(fixnum(result) == 100 && "We can lookup a previously defined variable");

    prog = STR(
            (begin
                    (define (sqr y)
                        (* y y))
                    (sqr 5))
    );
    exp = lisp_read(&prog);

    result = eval(exp, global_env);
    assert_ctr(fixnum(result) == 25 && "We can define a function and use it");

    lisp_cleanup();
}

void test_eval_environment() {
    lisp_init();
    cell params, result;

    params = mklist(2, mksym("+"), mkfixnum(3));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 3) && "Calling + with 1 param");

    params = mklist(3, mksym("+"), mkfixnum(2), mkfixnum(4));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 6) && "Calling + with 2 params");

    params = mklist(6, mksym("+"), mkfixnum(3), mkfixnum(3), mkfixnum(3), mkfixnum(3), mkfixnum(3));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 15) && "Calling + with n params");

    params = mklist(2, mksym("-"), mkfixnum(3));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 3) && "Calling - with 1 param");

    params = mklist(3, mksym("-"), mkfixnum(5), mkfixnum(2));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 3) && "Calling - with 2 params");

    params = mklist(6, mksym("-"), mkfixnum(12), mkfixnum(4), mkfixnum(4), mkfixnum(4), mkfixnum(2));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == -2) && "Calling - with n params");

    params = mklist(2, mksym("*"), mkfixnum(3));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 3) && "Calling * with 1 param");

    params = mklist(3, mksym("*"), mkfixnum(5), mkfixnum(2));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 10) && "Calling * with 2 params");

    params = mklist(6, mksym("*"), mkfixnum(1), mkfixnum(2), mkfixnum(3), mkfixnum(4), mkfixnum(5));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 120) && "Calling * with n params");

    params = mklist(2, mksym("/"), mkfixnum(2));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 2) && "Calling / with 1 param");

    params = mklist(3, mksym("/"), mkfixnum(10), mkfixnum(2));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 5) && "Calling / with 2 params");

    params = mklist(6, mksym("/"), mkfixnum(120), mkfixnum(2), mkfixnum(2), mkfixnum(2), mkfixnum(3));
    result = eval(params, global_env);
    assert_ctr((fixnum(result) == 5) && "Calling / with n params");

    lisp_cleanup();
}

cell adder(cell parms) {
    cell dee = first(parms);
    cell dum = car(rest(parms));

    if (nullp(rest(parms))) return mkfixnum(fixnum(dee));
    return  mkfixnum(fixnum(dee) + fixnum(dum));
}

cell square(cell parms) {
    cell num = car(parms);
    return mkfixnum(fixnum(num) * fixnum(num));
}

cell moreparms(cell parms) {
    return mkfixnum(lisp_length(parms));
}

void test_eval_primitive() {
    lisp_init();

    cell params, result;
    cell v = mksym("x");
    cell plus = mkfn("+", &adder);
    cell ad = mksym("+");
    cell sqr = mkfn("square", &square);
    define_variableb(ad, plus, global_env);

    params = mklist(3, ad, mkfixnum(1), mkfixnum(2));
    result = eval(params, global_env);
    assert_ctr( fixnum(result) == 3 && "Eval applies parms to primitive");

    // define variable x, call + on x x x
    define_variableb(v, mkfixnum(42), global_env);
    params = mklist(3, ad, v, v);
    result = eval(params, global_env);
    assert_ctr( fixnum(result) == 84 && "Eval applies variable parms to primitive");

    lisp_cleanup();
}

void test_make_functions() {
    lisp_init();
    cell plus = mkfn("+", &adder);
    cell sqr = mkfn("square", &square);
    cell more = mkfn("more", &moreparms);
    cell exp, result;

    assert_ctr(primitive_procp(plus) && "Should be a primitive");
    assert_ctr(primitive_procp(sqr) && "Should be a primitive");
    assert_ctr(lisp_eq(primitive_name(plus), "+") && "Name should be +");
    assert_ctr(lisp_eq(primitive_name(sqr), "square") && "Name should be square");

    exp = mklist(2, mkfixnum(2), mkfixnum(4));
    result = primitive_call(plus, exp);
    assert_ctr((fixnum(result) == 6) && "Calling plus should return correctly");

    exp = mklist(1, mkfixnum(3));
    result = primitive_call(sqr, exp);
    assert_ctr((fixnum(result) == 9) && "Calling square should return correctly");

    exp = mklist(5, mkfixnum(3), mkfixnum(3), mkfixnum(3), mkfixnum(3), mkfixnum(3));
    result = primitive_call(more, exp);
    assert_ctr((fixnum(result) == 5) && "Calling with n parms should work fine");

    lisp_cleanup();
}

void test_map_reduce() {
    lisp_init();
    cell exp, result;
    cell plus = mkfn("+", &adder);
    cell sqr = mkfn("square", &square);

    exp = mklist(5, mkfixnum(2), mkfixnum(2), mkfixnum(2), mkfixnum(2), mkfixnum(2));

    result = map(sqr, exp);
    assert_ctr(lisp_length(result)    == 5 && "Map returns a list");
    assert_ctr(fixnum(first(result))  == 4 && "first element is squared");
    assert_ctr(fixnum(second(result)) == 4 && "first element is squared");
    assert_ctr(fixnum(third(result))  == 4 && "first element is squared");
    assert_ctr(fixnum(nth(result, 4)) == 4 && "first element is squared");
    assert_ctr(fixnum(nth(result, 5)) == 4 && "first element is squared");

    result = reduce(plus, exp);
    assert_ctr((fixnum(result) == 10) && "Reduce a list to a result");

    lisp_cleanup();
}

void test_eval_apply() {
    lisp_init();
    cell params, result;
    cell v = mksym("x");
    cell plus = mkfn("+", &adder);
    cell sqr = mkfn("square", &square);
    cell more = mkfn("more", &moreparms);

    params = mklist(2, mkfixnum(1), mkfixnum(1));
    result = apply(plus, params);
    assert_ctr(fixnum(result) == 2 && "Applies 2 parms to primitive");

    params = mklist(1, mkfixnum(3));
    result = apply(sqr, params);
    assert_ctr(fixnum(result) == 9 && "Applies 1 parm to primitive");

    params = mklist(5, mkfixnum(2), mkfixnum(2), mkfixnum(2), mkfixnum(2), mkfixnum(2));
    result = apply(more, params);
    assert_ctr(fixnum(result) == 5 && "Applies n parm to primitive");
    lisp_cleanup();
}

void test_eval_cond() {
    lisp_init();
    cell exp, result;
    exp = mklist(4, mksym(COND), mkfixnum(1), quote(mksym("YES")), quote(mksym("NO")));
    result = eval(exp, global_env);
    assert_ctr( lisp_eq(result, "YES") && "True path followed");

    lisp_cleanup();
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

    lisp_cleanup();
}

void test_mklist() {
    cell exp;

    exp = mklist(3, mksym("set!"), mksym("x"), mkfixnum(42));
    assert_ctr((lisp_length(exp)) == 3 && "Should be a list of length 3");
    assert_ctr(lisp_equals(car(exp), mksym("set!")) && "first should be symbol set!");
    assert_ctr(lisp_equals(cadr(exp), mksym("x")) && "next should be should be symbol x");
    assert_ctr(lisp_equals(caddr(exp), mkfixnum(42)) && "finally should be should a fixnum of 42");

    exp = mklist(5, mksym("x"), mksym("y"), mksym("z"), mkfixnum(42), mkfixnum(24));
    assert_ctr((lisp_length(exp)) == 5 && "Should be a list of length 5");
    assert_ctr(lisp_eq(car(exp), "x") && "first should be symbol x");
    assert_ctr(lisp_eq(cadr(exp), "y") && "first should be symbol y");
    assert_ctr(lisp_eq(caddr(exp), "z") && "first should be symbol z");
    assert_ctr(lisp_equals(cadddr(exp), mkfixnum(42)) && "finally should be should a fixnum of 42");
    assert_ctr(lisp_equals(nth(exp, 5), mkfixnum(24)) && "finally should be should a fixnum of 24");

}

void test_eval_quoted() {
    lisp_init();
    cell exp, result;

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

    exp = mkfixnum(345);
    result = eval( quote(mklist(2, mkfixnum(6), mkfixnum(7))), nil );
    assert_ctr(listp(result) && "test_eval() :: quote should return a list");
    assert_ctr(lisp_equals(first(result), mkfixnum(6)) && "test_eval() :: quote should return a list");
    assert_ctr(lisp_equals(second(result), mkfixnum(7)) && "test_eval() :: quote should return a list");

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
    cell exp, exp1;
    const char *ptr;

    ptr = "'A";
    exp = lisp_read(&ptr);
    assert_ctr(car(exp) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(car(exp), "quote") && "test_lisp_quote() :: Symbol should be a quote");
    assert_ctr(cadr(exp) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(cadr(exp),"A") && "test_lisp_quote() :: Parsing should not be finished");

    ptr = "'(A)";
    exp = lisp_read(&ptr);
    assert_ctr(car(exp) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(car(exp), "quote") && "test_lisp_quote() :: Symbol should be a quote");
    assert_ctr(listp(cadr(exp)));
    assert_ctr(caadr(exp) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(caadr(exp),"A") && "test_lisp_quote() :: Parsing should not be finished");

    ptr = "'(A A)";
    exp = lisp_read(&ptr);
    assert_ctr(car(exp) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a a)");
    assert_ctr(lisp_eq(car(exp), "quote") && "test_lisp_quote() :: Symbol should be a quote");
    assert_ctr(listp(cadr(exp)));
    assert_ctr(caadr(exp) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a a)");
    assert_ctr(lisp_eq(caadr(exp),"A") && "test_lisp_quote() :: Parsing should not be finished");
    assert_ctr(cadadr(exp) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a a)");
    assert_ctr(lisp_eq(cadadr(exp),"A") && "test_lisp_quote() :: Parsing should not be finished");

    ptr = "('A 'B)";
    exp = lisp_read(&ptr);
    assert_ctr(listp(car(exp)));

    exp1 = car(exp);
    assert_ctr(car(exp1) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(car(exp1), "quote") && "test_lisp_quote() :: Symbol should be a quote");
    assert_ctr(cadr(exp1) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(cadr(exp1), "A") && "test_lisp_quote() :: Symbol should be a quote");

    exp1 = cadr(exp);
    assert_ctr(car(exp1) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(car(exp1), "quote") && "test_lisp_quote() :: Symbol should be a quote");
    assert_ctr(cadr(exp1) -> type == SYM && "test_lisp_quote() :: should return a quoted symbol (quote a)");
    assert_ctr(lisp_eq(cadr(exp1), "B") && "test_lisp_quote() :: Symbol should be a quote");

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

void test_eval_define() {
    lisp_init();

    cell v, result;
    v = mksym("x");
    define_variableb(v, mkfixnum(42), global_env);
    result = lookup_variable_value(v, global_env);
    assert_ctr( lisp_equals(result, mkfixnum(42)) && "Var x == 42");

    result = lookup_variable_value(mksym("true"), global_env);
    assert_ctr( lisp_equals(result, mkfixnum(1)) && "true should be defined");

    lisp_cleanup();
}

void test_eval_assignment() {
    lisp_init();
    cell v, result, exp;
    v = mksym("x");

    exp = mklist(3, mksym("set!"), v, mkfixnum(10));
    result = eval(exp, global_env);
    assert_ctr(result->type == ERROR && "Cannot assign unbound variable");

    define_variableb(v, mkfixnum(42), global_env);
    result = lookup_variable_value(v, global_env);
    assert_ctr( lisp_equals(result, mkfixnum(42)) && "Var x == 42");

    exp = mklist(3, mksym("set!"), v, mkfixnum(101));
    result = eval(exp, global_env);
    assert_ctr(result->type != ERROR && "Cannot assign unbound variable");
    result = lookup_variable_value(v, global_env);
    assert_ctr( lisp_equals(result, mkfixnum(101)) && "Var x updated to 101");
    lisp_cleanup();
}

void test_eval_variablep() {
    lisp_init();
    cell v, result;
    v = mksym("x");
    define_variableb(v, mkfixnum(42), global_env);
    result = eval(v, global_env);
    assert_ctr( lisp_equals(result, mkfixnum(42)) && "Var x == 42");
    result = eval(mksym("true"), global_env);
    assert_ctr( lisp_equals(result, mkfixnum(1)) && "Var x == 42");
    lisp_cleanup();
}

void test_eval_if() {
    lisp_init();
    cell exp, result;
    exp = mklist(4, mksym(IF), mkfixnum(1), quote(mksym("YES")), quote(mksym("NO")));
    result = eval(exp, global_env);
    assert_ctr( lisp_eq(result, "YES") && "True path followed");

    exp = mklist(3, mksym(IF), mkfixnum(1), quote(mksym("YES")));
    result = eval(exp, global_env);
    assert_ctr( lisp_eq(result, "YES") && "Only consequent specified, positive outcome" );

    exp = mklist(4, mksym(IF), mksym(FALSE), quote(mksym("YES")), quote(mksym("NO")));
    result = eval(exp, global_env);
    assert_ctr( lisp_eq(result, "NO") && "False path followed" );

    exp = mklist(4, mksym(IF), nil, quote(mksym("YES")), quote(mksym("NO")));
    result = eval(exp, global_env);
    assert_ctr( lisp_eq(result, "NO") && "False path followed" );

    exp = mklist(3, mksym(IF), mksym(FALSE), quote(mksym("YES")));
    result = eval(exp, global_env);
    assert_ctr( nullp(result) && "Only consequent specified, negative outcome" );

    lisp_cleanup();
}

void test_eval_lambda() {
    lisp_init();
    cell exp, result;
    exp = mklist(3, mksym(LAMBDA), mklist(1, mksym("x")), mksym("x"));
    result = eval(exp, global_env);
    assert_ctr( compound_procp(result) && "lambda returns a proc" );
    lisp_cleanup();
}

void test_eval_begin() {
    lisp_init();
    cell exp, result;
    exp = mklist(2, mksym(BEGIN), quote(mksym("y")));
    result = eval(exp, global_env);
    assert_ctr( lisp_eq(result, "y") && "begin returns result of its contents" );
    exp = mklist(2, mksym(BEGIN), mkfixnum(22));
    result = eval(exp, global_env);
    assert_ctr( fixnum(result) == 22 && "begin returns result of its contents" );
    exp = mklist(4, mksym(BEGIN), mkfixnum(22), mkfixnum(23), mkfixnum(42));
    result = eval(exp, global_env);
    assert_ctr( fixnum(result) == 42 && "begin returns last result of many expressions" );

    lisp_cleanup();
}