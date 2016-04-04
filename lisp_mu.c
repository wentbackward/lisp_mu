#include "lisp_mu.h"

const char * ERR_SYMTOOLONG = "Symbol length too long";
const char * ERR_LISTNOTTERMINATED = "List was not terminated";

#ifdef DEBUG
char * types[] = {"NIL","CONS","FIXNUM","FLOAT","STRING","SYM","ERROR"};
#endif


/**
 * ----------------------------------------------------------------------
 * Garbage Collector
 * ----------------------------------------------------------------------
 * The garbage collect uses the following algo:
 * 1. All list objects are stored in a single list `all_objects'
 * 2. Sweep will scan `all_symbols'
 * 3. recursively mark referenced objects as accessible
 * 4. All unmarked objects as inaccessible and thus can be reclaimed
 *
 * Notes:
 * There is no automatic GC, it is initiated by the host. This is important for
 * devices operating in real-time. This means the responsibility for memory
 * management is on the host, thus the GC may be considered only partuial GC
 *
 * Special objects must be protected, these are created by lisp_init and cleaned up by lisp_cleanup
 * e.g. nil, all_objects, all_symbols
 *
 */
bool lisp_sweep() {

}

bool lisp_free(bool force_clean_all) {
    cell ptr = all_objects;
    if (force_clean_all) {
#ifdef DEBUG
        puts("Delete all objects");
#endif
        while(!nullp(ptr)) {
#ifdef DEBUG
            puts("=======================");
            printf("Address to destroy %d, type:%s\n", car(ptr), types[car(ptr)->type]);
            printf("Name of object: %s\n", car(ptr)->name);
            printf("all_o: %d, nil: %d, all_sym: %d\n", all_objects, nil, all_symbols);
            printf("Length of all_objects before %d\n", lisp_length(all_objects));
            lisp_pprint(all_objects, 0);
#endif
            cell tmp = cdr(ptr);
            lisp_destroy(car(ptr), all_objects);
            ptr = tmp;
#ifdef DEBUG
            printf("Length of all_objects after %d\n", lisp_length(all_objects));
#endif
        }
    }

    // Check if !marked

}

cell find_object(cell address, cell objects) {
    if (!listp(objects)) return nil;
    cell o = objects;
    cell result = nil;
    while(!nullp(o)) {
        cell test = car(o);
        if (test == address) {
            result = o;
            break;
        }
        o = rest(o);
    }
    return result;
}


/**
 * ----------------------------------------------------------------------
 * Evaluation
 */

cell eval(cell exp, cell env) {
    if (self_evaluatingp(exp)) return exp;
    if (variablep(exp)) return lookup_variable_value(exp, env);
}

cell lookup_variable_value(cell var, cell env) {
    return nil;
}


bool self_evaluatingp(cell exp) {
    return (exp->type == STRING || numberp(exp));
}

bool variablep(cell exp) {
    return symbolp(exp);
}

/**
 * ----------------------------------------------------------------------
 * Parsing and basic lisp list processing
 */


int lisp_length(cell exp) {
    if (nullp(exp)) return 0;
    if (!listp(exp)) return 0;
    return (1 + lisp_length(rest(exp)));
}

/*
 * Find the last element of a list
 */
cell last_aux(cell exp) {
    if (nullp(exp->rest)) return exp;
    return last_aux(exp->rest);
}
cell last(cell list) {
    if (nullp(list)) { return nil; }        // No list
    if (!listp(list)) { return nil; }       // Not a list
    if (nullp(list->adata)) { return nil; } // Empty list
    return last_aux(list->adata);
}

cell nth(cell exp, int n) {
    if (nullp(exp)) return nil;
    if (n == 1) return car(exp);
    return nth(cdr(exp), --n);
}

/*
 * Append to a list (to the end of the list)
 */
cell append(cell head, cell other) {
    cell list = head;
    if (nullp(head))
        list = cons(other, nil);
    else
        last(list)->rest = other;
    return list;
}

bool nullp(cell exp) { lisp_equals(exp, nil); }
bool listp(cell exp) {
    if (nullp(exp)) return true;
    return (exp->type == CONS);
}

bool numberp(cell exp) {
    switch (exp->type) {
        case FIXNUM:
        case FLOAT:
            return true;
        default:
            return false;
    }
}

bool symbolp(cell exp) {
    return (exp->type == SYM);
}

/**
 * Compares a lisp expression with a raw data type
 */
bool lisp_eq(cell lhs, any rhs) {
    bool result = false;

    switch (lhs->type) {
        case NIL:
            result = ((lhs -> data == nil) && (rhs == nil || rhs == NULL));
            break;
        case FIXNUM:
            result = ( fixnum(lhs) == *((lisp_fixnum *)rhs) );
            break;
        case ERROR:
        case STRING:
        case SYM:
            result = ( strcmp(symbol(lhs), (lisp_char *)rhs) == 0);
            break;
#ifdef WITH_FLOATING_POINT
        case FLOAT:
            result = ( floater(lhs) == *((lisp_float *)rhs) );
            break;
#endif
        case CONS:
            result = (lhs == rhs);
    }
    return result;
}

/**
 * Compare two lisp expressions
 */
bool lisp_equals(cell lhs, cell rhs) {
    bool result = false;
    if (lhs == rhs) return true;
    if (lhs->type != rhs->type) return false;

    switch (lhs->type) {
        case NIL:
            result = (rhs -> type == NIL);
            break;
        case FIXNUM:
            result = ( fixnum(lhs) == fixnum(rhs) );
            break;
#ifdef WITH_FLOATING_POINT
        case FLOAT:
            result = ( floater(lhs) == floater(rhs) );
            break;
#endif
        case STRING:
        case ERROR:
        case SYM:
            result = ( strcmp(symbol(lhs), symbol(rhs)) == 0 );
            break;
        case CONS:
            result = ( lhs == rhs );
    }
    return result;
}


/**
 * ----------------------------------------------------------------------
 * Make lisp objects subsystem
 */


cell primary_alloc(enum lisp_type type, size_t length, any data, cell rest) {
    cell result = calloc(1, sizeof(lisp_cell));
    result->type = type;
    result->marked_for_gc = false;
    result->length = length;
    result->data = data;
    result->rest = rest;

    return result;
}

cell lisp_alloc(enum lisp_type type, size_t length, any data, cell rest) {
    cell result = primary_alloc(type, length, data, rest);

    if (length == 0 || data == NULL || data == nil) {
        result->data = nil;
        result->length = lisp_sizeof(CONS);
    } else {
        switch (type) {
            case NIL:
            case CONS:
                break;
            case FIXNUM:
            case FLOAT:
            case STRING:
            case SYM:
            case ERROR: {
                any ptr = malloc(length);
                memcpy(ptr, data, length);
                result->data = ptr;
                break;
            }
        }
    }

    if (rest == NULL)
        result->rest = nil;

    if(rest != all_objects || all_objects == nil)
        all_objects = cons(result, all_objects);

    return result;
}

bool destroy_aux(cell exp, cell objects, cell last) {
    if (nullp(objects)) return false;  // Searched past end of list
    if(exp == nil || exp == all_symbols || exp == env) return false;

    // Is the first element of `objects' the cell we're looking for?
    if (car(objects) == exp) {
        if (exp->type != CONS && exp->type != NIL)
            free(exp->data);
        free(exp);                  // Free the thing we're looking for

        if (objects == last) {
            all_objects = cdr(objects); // Change the head of the heap to be the next item
            free(objects);              // Free the previous head of the heap
            return false;
        } else {
            return true;
        }
    }

    if (destroy_aux(exp, cdr(objects), objects)) {
        // It was destroyed, now slice it from the heap
        setcdr(objects, cdr(cdr(objects)));  // Set this object to the next next
        free(cdr(objects));                  // Release the next
    }
}

void lisp_destroy(cell exp, cell objects) {
    // Cannot cleanup specials here, but can cleanup nil objects
    if(exp == nil || exp == all_symbols || exp == env) return;
    if (nullp(objects)) return;

    destroy_aux(exp, objects, objects);
}

size_t string_size(size_t chars) { return sizeof(lisp_char) * (chars + 1); }
cell cons(cell x, cell y)           { return lisp_alloc(CONS, lisp_sizeof(CONS), x, y); }
cell mkfixnum(lisp_fixnum l)        { return lisp_alloc(FIXNUM, sizeof(lisp_fixnum), &l, nil); }
cell mksym(lisp_char * exp)         { return lisp_alloc(SYM, string_size(strlen(exp)), exp, nil); }
cell mkerror(const char *errmsg)    { return lisp_alloc(ERROR, string_size(strlen(errmsg)), (any) errmsg, nil); }
cell mkstring(lisp_char * str)      { return lisp_alloc(STRING, string_size(strlen(str)), str, nil); }
cell mkquote()                      { return mksym("'");}

#ifdef WITH_FLOATING_POINT
cell mkfloat(lisp_float f) { return lisp_alloc(FLOAT, sizeof(lisp_float), &f, nil); }
#endif

cell lisp_read_list(const char **buf) {
    cell last = nil;
    cell tmp  = nil;
    cell list = nil;

    while(**buf) {
        if (isspace(**buf)) {
            ++*buf;
            continue;
        }

        switch(**buf) {
            case '\'':
                ++*buf;
                tmp = mkquote();
                break;
            case ')':
                ++*buf;
                return list;
            case '(':
                ++*buf;
                tmp = lisp_read_list(buf);
                break;
            case '"':
                ++*buf;
                tmp = lisp_read_string(buf);
                break;
            default:
                tmp = lisp_read_symbol(buf);
                break;
        }

        tmp = cons(tmp, nil);
        if (nullp(list)) {
            last = list = tmp;
        } else {
            last->rest = tmp;
            last = tmp;
        }
    }

    return mkerror(ERR_LISTNOTTERMINATED);
}



cell lisp_read_symbol(const char **buf) {
    cell result;
    char * endp;
    char data[MAXLEN] = {0};
    int i = 0;

    while(**buf) {
        if (i >= MAXLEN) {
            result = mkerror(ERR_SYMTOOLONG);
            break;
        }
        if (isspace(**buf)) {
            ++*buf;
            break;
        }
        if (**buf == ')')
            break;

        data[i] = **buf;
        ++*buf;
        ++i;
    }

    lisp_fixnum l = strtol(data, &endp, 0);
    if (data != endp && *endp == '\0') {
        result = mkfixnum(l);
    } else {
#ifdef WITH_FLOATING_POINT
        lisp_float d = strtod(data, &endp);
        if (data != endp && *endp == '\0') {
            result = mkfloat(d);
        } else {
            result = mksym(data);
        }
#else
        result = mksym(data);
#endif
    }
    return result;
}

cell lisp_read_string(const char **buf) {
    cell result;
    lisp_char *data = calloc(MAXLEN, sizeof(lisp_char));
    int fact = 1;
    int i = 0;
    bool escaped = false;

    while(**buf) {
        if (i >= MAXLEN) data = realloc(data, ++fact * MAXLEN * sizeof(lisp_char));
        escaped = (**buf == '\\');

        if (**buf == '"' && !escaped) {
            data[i] = 0;
            break;
        }

        if (escaped) ++*buf;
        data[i] = **buf;

        ++*buf;
        ++i;
    }

    result = mkstring(data);
    free(data);
    return result;
}

cell lisp_read(const char **buf) {
    while(**buf) {
        if (isspace(**buf)) {
            ++*buf;
            continue;
        }
        switch (**buf) {
            case '\'':
                ++*buf;
                return mkquote();
            case '(':
                ++*buf;
                return lisp_read_list(buf);
            case '"':
                ++*buf;
                return lisp_read_string(buf);
            default:
                return lisp_read_symbol(buf);
        }
    }
    return nil;
}

void lisp_init() {
    nil         = primary_alloc(NIL, lisp_sizeof(CONS), NULL, NULL);
    setcar(nil, nil); setcdr(nil, nil);
    all_objects = primary_alloc(CONS, lisp_sizeof(CONS), nil, nil);

#ifdef DEBUG
    nil->name = "NIL";
    all_objects->name = "ALL_OBJECTS";
    all_symbols->name = "ALL_SYMBOLS";
    env->name = "ENV";
#endif
}

void lisp_cleanup() {
    // Calling lisp_free with cleanup all set to true
    lisp_free(true);

    // These special symbols must be freed explicitly
    free(all_objects);
    free(nil);
}

size_t lisp_sizeof(enum lisp_type type) {
    size_t result = 0;

    switch (type) {
        case CONS:
            result = sizeof(lisp_cell);
            break;
        case FIXNUM:
            result = sizeof(lisp_fixnum);
            break;
#ifdef WITH_FLOATING_POINT
        case FLOAT:
            result = sizeof(lisp_float);
            break;
#endif
        case ERROR:
        case STRING:
        case SYM:
            result = sizeof(lisp_char);
            break;
        case NIL:
            result = 0;
            break;
    }
    return result;
}


void lisp_print_list(cell i) {
    if (!listp(i)) { return; }
    cell ptr = i;
    printf("(");
    while(!nullp(ptr)) {
        cell e = car(ptr);
        switch(e->type) {
            case NIL:
                printf("NIL");
                break;
            case CONS:
                printf("<#LIST: ?>");
                break;
            case STRING:
                printf("<#STRING: \"%s\">", e->string);
                break;
            case FIXNUM:
                printf("<#FIXNUM: %d>", (int) fixnum(e));
                break;
#ifdef WITH_FLOATING_POINT
            case FLOAT:
                printf("<#FLOAT: %f>", floater(e));
                break;
#endif
            case SYM:
                printf("<#SYM: %s>", symbol(e));
                break;
            case ERROR:
                printf("<#ERROR: \"%s\">", e->string);
                break;
        }
        printf(" ");
        ptr = rest(ptr);
    }
    printf(")\n");
}

void lisp_pprint(cell e, int depth) {
#define sep() puts(""); for(int i = 0; i < depth; i++) printf("  ")

    switch(e->type) {
        case NIL:
            printf("NIL ");
            break;
        case CONS:
            if (!nullp(e)) {
                sep();
                printf("(");
                sep();
                lisp_pprint(car(e), depth + 1);
                printf(" . ");
                lisp_pprint(cdr(e), depth + 1);
                sep();
                printf(")\n");
            } else {
            }
            break;
        case STRING:
            printf("String: \"%s\"", e->string);
            break;
        case FIXNUM:
            printf("Fixnum: %d ", (int) fixnum(e));
            break;
#ifdef WITH_FLOATING_POINT
        case FLOAT:
            printf("Float: %f ", floater(e));
            break;
#endif
        case SYM:
            printf("Symbol: %s ", symbol(e));
            break;
        case ERROR:
            printf("Error: \"%s\"", e->string);
            break;
    }
}
