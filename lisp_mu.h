#ifndef __LISP_MU__
#define __LISP_MU__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// Generally on micro controllers, using doubles/floats will bloat the codebase, comment this to
// remove double support at compile time
#define WITH_FLOATING_POINT
//#define DEBUG

#define MAXLEN 256  // max length of strings and symbols

#define QUOTE       "quote"
#define SETB        "set!"
#define DEFINE      "define"
#define LAMBDA      "lambda"
#define IF          "if"
#define BEGIN       "begin"
#define COND        "cond"
#define ELSE        "else"
#define FALSE       "false"
#define OK          "ok"
#define PROC        "procedure"
#define PRIMITIVE   "primitive"

// Here you can affect the underlying data types used in the lisp system
#ifdef WITH_FLOATING_POINT
typedef double  lisp_float;
#endif
typedef long    lisp_fixnum;
typedef char    lisp_char;
typedef void    *any;
enum lisp_type {
    NIL, CONS, FIXNUM, FLOAT, STRING, SYM, ERROR
};

typedef struct cell {
    enum lisp_type type;
#ifdef DEBUG
    char * name;
#endif
    struct cell *rest;
    bool marked_for_gc;
    size_t length;
    union {
        any              data;
        struct cell     *adata;
        lisp_fixnum     *fixnum;
        lisp_char       *string;
        lisp_char       *symbol;
#ifdef WITH_FLOATING_POINT
        lisp_float      *floater;
#endif
    };
} lisp_cell, *cell;

cell nil, all_objects, the_empty_environment;

// Accessors
#define fixnum(A)       *((A)->fixnum)
#define symbol(A)       ((A)->symbol)
#define car(A)          ((A)->adata)
#define first(A)        car(A)
#define setcarb(A,B)     ((A)->adata = (B))
#define cdr(A)          ((A)->rest)
#define rest(A)         cdr(A)
#define setcdrb(A,B)     ((A)->rest = (B))
#define second(A)       car(cdr(A))
#define third(A)        car(cdr(cdr(A)))
#define cadr(A)         car(cdr(A))
#define cddr(A)         cdr(cdr(A))
#define caddr(A)        car(cdr(cdr(A)))
#define caadr(A)        car(car(cdr(A)))
#define cdadr(A)        cdr(car(cdr(A)))
#define cadddr(A)       car(cdr(cdr(cdr(A))))

// Special handling of doubles
#ifdef WITH_FLOATING_POINT
#define floater(A)      *((A)->floater)

cell mkfloat(lisp_float f);
#endif

// Evaluation subsystem
// See: https://mitpress.mit.edu/sicp/full-text/book/book-Z-H-26.html
cell eval(cell exp, cell env);
cell apply(cell procedure, cell arguments);
bool self_evaluatingp(cell exp);
bool falsep(cell exp);
bool truep(cell exp);
cell eval_if(cell exp, cell env);
cell eval_sequence(cell exps, cell env);
cell eval_assignment(cell exp, cell env);
cell eval_definition(cell exp, cell env);

// Environment
#define enclosing_environment(A)    cdr(A)
#define first_frame(A)              car(A)
#define mkframe(A,B)                cons(A,B)
#define frame_variables(A)          car(A)
#define frame_values(A)             cdr(A)
cell add_binding_to_frameb(cell var, cell val, cell frame);
cell extend_environment(cell vars, cell vals, cell base_env);

// Quotes
#define quotedp(A)                  tagged_listp(A, QUOTE)

// Procedure
cell mkprocedure(cell parameters, cell body, cell env);
#define compound_procp(A)           tagged_listp(A, PROC)
#define procedure_parameters(A)     cadr(A)
#define procedure_body(A)           caddr(A)
#define procedure_environment(A)    cadddr(A)
cell list_of_values(cell exp, cell env);

// Primitives
#define primitive_procp(A)          tagged_listp(A, PRIMITIVE)

// Assignment variables
#define assignmentp(A)              tagged_listp((A), SETB)
#define assignment_variable(A)      cadr(A)
#define assignment_value(A)         caddr(A)
bool variablep(cell exp);
cell lookup_variable_value(cell var, cell env);
cell set_variable_valueb(cell var, cell val, cell env);
cell define_variableb(cell var, cell val, cell env);

// Definition variables
#define definitionp(A)              tagged_listp(A, DEFINE)
cell definition_variable(cell exp);
cell definition_value(cell exp);

// Lambda
#define lambdap(A)                  tagged_listp(A, LAMBDA)
#define lambda_parameters(A)        cadr(A)
#define lambda_body(A)              cddr(A)
#define mklambda(P, B)              cons(mksym(LAMBDA), cons(P,B))

// Conditionals
#define ifp(A)                      tagged_listp(A, IF)
#define if_predicate(A)             cadr(A)
#define if_consequent(A)            caddr(A)
cell if_alternate(cell exp);
cell mkif(cell predicate, cell consequent, cell altnerate);

// Begin
#define beginp(A)                   tagged_listp(A, BEGIN)
#define begin_actions(A)            cdr(A)
#define last_expp(A)                nullp(cdr(A))
#define first_exp(A)                car(A)
#define rest_exps(A)                cdr(A)
cell sequence_exp(cell exp);
cell mkbegin(cell seq);


// Application
#define applicationp(A)             pairp(A)
#define operator(A)                 car(A)
#define operands(A)                 cdr(A)
#define no_operandsp(A)             nullp(A)
#define first_operand(A)            car(A)
#define rest_operands(A)            cdr(A)


// Cond, seeing as SICP authors have done all the hard work, we can implement in C for speed and
// also to reduce the size of the lisp programs
#define condp(A)                    tagged_listp(A, COND)
#define cond_clauses(A)             cdr(A)
#define cond_predicate(A)           car(A)
#define cond_elsep(A)               lisp_eq(cond_predicate(A), ELSE)
#define cond_actions(A)             cdr(A)
#define cond_if(A)                  expand_clauses(cond_clauses(A))
cell expand_clauses(cell clauses);

// Parsing subsystem
size_t lisp_sizeof(enum lisp_type);

bool lisp_equals(cell lhs, cell rhs);
bool lisp_eq(cell lhs, any rhs);
bool nullp(cell exp);
bool listp(cell exp);
bool numberp(cell exp);
bool symbolp(cell exp);
bool pairp(cell exp);

// Set up and tear down of lisp environment
void lisp_init();
void lisp_cleanup();
bool tagged_listp(cell exp, lisp_char *symbol);

// Parsers
cell lisp_read        (const char **);
cell lisp_read_symbol (const char **buf);
cell lisp_read_string (const char **buf);
cell lisp_read_list   (const char **buf);

void lisp_print_list(cell i);
void lisp_pprint(cell e, int depth);
cell last(cell list);
cell nth(cell list, int n);
int lisp_length(cell exp);

size_t string_size(size_t chars);
cell lisp_alloc   (enum lisp_type type, size_t length, any data, cell rest);
void lisp_destroy (cell, cell);
cell cons         (cell x, cell y);
cell mkfixnum     (lisp_fixnum f);
cell mksym        (lisp_char * exp);
cell mknil        ();
cell mkquote      ();
cell mkstring     (lisp_char * str);
cell mkerror      (const char *errmsg);
cell quote        (cell exp);

// Garbage Collector
bool lisp_sweep();
bool lisp_free(bool force_clean_all);
cell find_object(cell address, cell objects);


/*
#ifdef WITH_FLOATING_POINT
double sum_double(int numargs, ...);
#endif
*/



/*
int sum_int(int numargs, ...) {
    va_list ap;
    int result = 0;
    va_start(ap, numargs);
    for (int i = 0; i < numargs; ++i)
        result += va_arg(ap, int);
    va_end(ap);
    return result;
}

#ifdef WITH_FLOATING_POINT

double sum_double(int numargs, ...) {
    va_list ap;
    double result = 0;
    va_start(ap, numargs);
    for (int i = 0; i < numargs; ++i)
        result += va_arg(ap, double);
    va_end(ap);
    return result;
}

#endif

 */

#endif // __LISP_MU__