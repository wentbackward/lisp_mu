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

cell nil, all_objects, all_symbols, env;

// Accessors
#define fixnum(A)       *((A)->fixnum)
#define symbol(A)       ((A)->symbol)
#define car(A)          ((A)->adata)
#define first(A)        car(A)
#define setcar(A,B)     ((A)->adata = (B))
#define cdr(A)          ((A)->rest)
#define rest(A)         cdr(A)
#define setcdr(A,B)     ((A)->rest = (B))
#define second(A)       car(cdr(A))
#define third(A)        car(cdr(cdr(A)))


// Special handling of doubles
#ifdef WITH_FLOATING_POINT
#define floater(A)      *((A)->floater)
cell mkfloat(lisp_float f);
#endif

// Evaluation subsystem
cell eval(cell exp, cell env);
bool self_evaluatingp(cell exp);
bool variablep(cell exp);
cell lookup_variable_value(cell var, cell env);

// Parsing subsystem
size_t lisp_sizeof(enum lisp_type);

bool lisp_equals(cell lhs, cell rhs);
bool lisp_eq(cell lhs, any rhs);
bool nullp(cell exp);
bool listp(cell exp);
bool numberp(cell exp);
bool symbolp(cell exp);

// Set up and tear down of lisp environment
void lisp_init();
void lisp_cleanup();

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
