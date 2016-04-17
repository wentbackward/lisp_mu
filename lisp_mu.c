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
 *
 * See: https://mitpress.mit.edu/sicp/full-text/book/book-Z-H-26.html
 *
 */

cell eval(cell exp, cell env) {
    if (self_evaluatingp(exp))  return exp;
    if (variablep(exp))         return lookup_variable_value(exp, env);
    if (quotedp(exp))           return cadr(exp);
    if (assignmentp(exp))       return eval_assignment(exp, env);
    if (definitionp(exp))       return eval_definition(exp, env);
    if (ifp(exp))               return eval_if(exp, env);
    if (lambdap(exp))
        return mkprocedure(lambda_parameters(exp),
                           lambda_body(exp), env);
    if (beginp(exp))            return eval_sequence(begin_actions(exp), env);
    if (condp(exp))             return eval(cond_if(exp), env);
    if (applicationp(exp))
        return apply(eval(operator(exp), env),
                     (list_of_values(operands(exp), env)));
    return mkerror("Unknown expression type -- EVAL");
}

cell list_of_values(cell exps, cell env) {
    if (no_operandsp(exps))
        return nil;
    else
        return cons(eval(first_operand(exps), env),
                    list_of_values(rest_operands(exps), env));
}

bool falsep(cell exp) {
    return lisp_eq(exp, FALSE) || nullp(exp);
}

bool truep(cell exp) {
    return !falsep(exp);
}

cell eval_if(cell exp, cell env) {
    if (truep(eval(if_predicate(exp), env)))
        return eval(if_consequent(exp), env);
    else
        return eval(if_alternate(exp), env);
}

cell eval_sequence(cell exps, cell env) {
    if(last_expp(exps))
        return eval(first_exp(exps), env);
    else {
        eval(first_exp(exps), env);
        return eval_sequence(rest_exps(exps), env);
    }
}

cell eval_assignment(cell exp, cell env) {
    cell result = set_variable_valueb(assignment_variable(exp), eval(assignment_value(exp), env), env);
    if (result -> type == ERROR)
        return result;
    else
        return mksym(OK);
}

cell eval_definition(cell exp, cell env) {
    define_variableb(definition_variable(exp), eval(definition_value(exp), env), env);
    return mksym(OK);
}

cell mkprocedure(cell parameters, cell body, cell env) {
    return mklist(4, mksym(PROC), parameters, body, env);
}

cell add_binding_to_frameb(cell var, cell val, cell frame) {
    setcarb(frame, cons(var, car(frame)));
    return setcdrb(frame, cons(val, cdr(frame)));
}

cell setup_environment() {
    cell initial = extend_environment(nil, nil, the_empty_environment);
    define_variableb(mksym("true"), mkfixnum(1), initial);
    define_variableb(mksym("false"), nil, initial);
    return initial;
}

cell extend_environment(cell vars, cell vals, cell base_env) {
    if (lisp_length(vars) == lisp_length(vals))
        return cons(mkframe(vars, vals), base_env);
    else if (lisp_length(vars) < lisp_length(vals))
        return mkerror("Too many arguments supplied -- EXTEND_ENVIRONMENT");
    else
        return mkerror("Too few arguments supplied -- EXTEND_ENVIRONMENT");
}

cell lookup_variable_value(cell var, cell env) {
    cell env_loop(cell nenv) {
        cell scan(cell vars, cell vals) {
            if (nullp(vars))
                return env_loop(enclosing_environment(nenv));
            else if (lisp_equals(var, car(vars)))
                return car(vals);
            else
                return scan(cdr(vars), cdr(vals));
        }
        if (nenv == the_empty_environment)
            return mkerror("Unbound Variable -- lookup_variable_value");
        else {
            cell frame = first_frame(nenv);
            return scan(frame_variables(frame), frame_values(frame));
        }
    }
    return env_loop(env);
}

cell set_variable_valueb(cell var, cell val, cell env) {
    cell env_loop(cell nenv) {
        cell scan(cell vars, cell vals) {
            if (nullp(vars))
                return env_loop(enclosing_environment(nenv));
            else if (lisp_equals(var, car(vars)))
                return setcarb(vals, val);
            else
                return scan(cdr(vars), cdr(vals));
        }
        if (nenv == the_empty_environment)
            return mkerror("Unbound Variable -- set_variable_valueb");
        else {
            cell frame = first_frame(nenv);
            return scan(frame_variables(frame), frame_values(frame));
        }
    }
    return env_loop(env);
}

cell define_variableb(cell var, cell val, cell env) {
    cell frame = first_frame(env);

    cell scan(cell vars,cell vals) {
        if (nullp(vars))
            return add_binding_to_frameb(var, val, frame);
        else if (lisp_equals(var, car(vars)))
            return setcarb(vals, val);
        else
            return scan(cdr(vars), cdr(vals));
    }

    return scan(frame_variables(frame), frame_values(frame));
}

cell apply_primitive_proc(cell proc, cell args) {
    //(apply-in-underlying-scheme
    //(primitive-implementation proc) args))



    return mkerror("Unknown procedure type - APPLY PRIM");
}

cell apply(cell procedure, cell arguments) {
    if (primitive_procp(procedure)) {
        return primitive_call(procedure, arguments);
//    } else if (compound_procp(procedure)) {
//        return eval_sequence(
//                procedure_body(procedure),
//                (extend_environment(procedure_parameters(procedure),
//                  arguments, procedure_environment(procedure))));
    }
    return mkerror("Unknown procedure type - APPLY");
}



bool self_evaluatingp(cell exp) {
    return (exp->type == STRING || numberp(exp) || nullp(exp));
}

bool variablep(cell exp) {
    return symbolp(exp);
}

cell definition_variable(cell exp) {
    if(symbolp(cadr(exp)))
        return (cadr(exp));
    else
        return (caadr(exp));
}

cell definition_value(cell exp) {
    if(symbolp(cadr(exp)))
        return (caddr(exp));
    else
        return mklambda(cdadr(exp), cddr(exp));
}

cell if_alternate(cell exp) {
    if (!nullp(cdddr(exp)))
        return cadddr(exp);
    return nil;
}

cell mkif(cell predicate, cell consequent, cell altnerate) {
    return cons(mksym(IF), cons(predicate, cons(consequent, altnerate)));
}


cell mkbegin(cell seq) {
    return cons(mksym(BEGIN), seq);
}

cell sequence_exp(cell seq) {
    if (nullp(seq))
        return seq;
    else if (last_expp(seq))
        return first_exp(seq);
    else
        return mkbegin(seq);
}

cell expand_clauses(cell clauses) {
    if (nullp(clauses)) {
        return nil;
    }
    cell first = car(clauses);
    cell rest = cdr(clauses);

    if (cond_elsep(first)) {
       if (nullp(rest)) {
           return sequence_exp(cond_actions(first));
       } else {
           return mkerror("ELSE clause isn't last cond-if");
       }
    } else {
        return mkif(cond_predicate(first),
                sequence_exp(cond_actions(first)),
                expand_clauses(rest));
    }
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
        case FN:
            result = (lhs == rhs);
    }
    return result;
}

cell mklist(int l, ...) {
    va_list ap;
    bool first = true;
    cell exp;
    cell list=nil;
    cell c=nil;
    cell last=nil;
    va_start(ap,l);
    for(; l>0; l--) {
        exp = va_arg(ap, cell);
        c = cons(exp, nil);
        if (first) {
            first = false;
            list = last = c;
        } else {
            setcdrb(last, c);
            last = c;
        }
    }
    setcdrb(last, nil);
    va_end(ap);
    return list;
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
        case FN:
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
//    result->marked_for_gc = false;
//    result->length = length;
    result->data = data;
    result->rest = rest;

    return result;
}

cell lisp_alloc(enum lisp_type type, size_t length, any data, cell rest) {
    cell result = primary_alloc(type, length, data, rest);

    switch (type) {
        case FN:
            result->data = data;
            break;
        default:
            if (length == 0 || data == NULL || data == nil) {
                result->data = nil;
//              result->length = lisp_sizeof(CONS);
            } else {
                switch (type) {
                    case FIXNUM:
                    case FLOAT:
                    case STRING:
                    case SYM:
                    case ERROR: {
                        any ptr = malloc(length);
                        memcpy(ptr, data, length);
                        result->data = ptr;
                        break;
                    default:
                        break;
                    }
                }
            }
            break;
    }


    if (rest == NULL)
        result->rest = nil;

    if(rest != all_objects || all_objects == nil)
        all_objects = cons(result, all_objects);

    return result;
}

bool destroy_aux(cell exp, cell objects, cell last) {
    if (nullp(objects)) return false;  // Searched past end of list
    if(exp == nil || exp == all_objects) return false;

    // Is the first element of `objects' the cell we're looking for?
    if (car(objects) == exp) {
        switch (exp->type) {
            case NIL:
            case CONS:
            case FN:
                break;
            default:
                free(exp->data);
                break;
        }
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
        setcdrb(objects, cdr(cdr(objects)));  // Set this object to the next next
        free(cdr(objects));                  // Release the next
    }
}

void lisp_destroy(cell exp, cell objects) {
    // Cannot cleanup specials here, but can cleanup nil objects
    if(exp == nil || exp == all_objects) return;
    if (nullp(objects)) return;

    destroy_aux(exp, objects, objects);
}

cell lisp_read_list(const char **buf) {
    cell last = nil;
    cell tmp  = nil;
    cell list = nil;
    bool doquote = false;

    while(**buf) {
        if (isspace(**buf)) {
            ++*buf;
            doquote = false;
            continue;
        }

        switch(**buf) {
            case '\'':
                doquote = true;
                ++*buf;
                break;
            case ')':
                ++*buf;
                return list;
            case '(':
                ++*buf;
                if (doquote) {
                    doquote = false;
                    tmp = quote(lisp_read_list(buf));
                } else
                    tmp = lisp_read_list(buf);

                break;
            case '"':
                ++*buf;
                if (doquote) {
                    doquote = false;
                    tmp = quote(lisp_read_string(buf));
                } else
                    tmp = lisp_read_string(buf);
                break;
            default:
                if (doquote) {
                    doquote = false;
                    tmp = quote(lisp_read_symbol(buf));
                } else
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
    bool doquote = false;
    while(**buf) {
        if (isspace(**buf)) {
            ++*buf;
            continue;
        }
        switch (**buf) {
            case '\'':
                ++*buf;
                doquote = true;
                break;
            case '(':
                ++*buf;
                if (doquote)
                    return quote(lisp_read_list(buf));
                else
                    return lisp_read_list(buf);
            case '"':
                ++*buf;
                return lisp_read_string(buf);
            default:
                if (doquote)
                    return quote(lisp_read_symbol(buf));
                else
                    return lisp_read_symbol(buf);
        }
    }
    return nil;
}

void lisp_init() {
    // Must be manually cleaned up
    nil          = primary_alloc(NIL, lisp_sizeof(CONS), NULL, NULL);
    setcarb(nil, nil); setcdrb(nil, nil);
    all_objects  = primary_alloc(CONS, lisp_sizeof(CONS), nil, nil);

    // Cleanup all will get these
    the_empty_environment = cons(nil, nil);
    global_env = setup_environment();

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
        case FN:
            result = 0;
            break;
    }
    return result;
}

cell mkfixnum(lisp_fixnum A)        { return lisp_alloc(FIXNUM, sizeof(lisp_fixnum), &A, nil); }
#ifdef WITH_FLOATING_POINT
cell mkfloat(lisp_float f) { return lisp_alloc(FLOAT, sizeof(lisp_float), &f, nil); }
#endif


/*
 * Primitives features
 */
cell map(cell fn, cell list) {
    cell c = list;
    cell result = nil;
    cell last = nil;
    cell tmp = nil;
    while(!nullp(c)) {
        if (primitive_procp(fn)) {
            tmp = cons(primitive_call(fn, c), nil);
        }
        if (nullp(result)) {
            last = result = tmp;
        } else {
            setcdrb(last, tmp);
            last = tmp;
        }
        c = cdr(c);
    }
    return result;
}

cell reduce(cell fn, cell list) {
    cell c = list;
    cell result = nil;
    while(true) {
        if (primitive_procp(fn)) {
            result = primitive_call(fn, c);
        }
        if (nullp(cdr(c))) break;
        c = cons(result, cddr(c));
    }
    return result;
}


void lisp_pprint(cell i) {
    lisp_print_list_aux(i, 0);
}

void lisp_print_list_aux(cell i, int depth) {
    #define sep() puts(""); for(int i = 0; i < depth; i++) printf("  ")
    bool first = true;
    cell ptr = i;
    sep();
    if (listp(i)) {
        printf("(");
    }
    while(!nullp(ptr)) {
        cell e = car(ptr);
        if (!first) {
            printf(" ");
        }
        first = false;
        switch(e->type) {
            case NIL:
                printf("NIL");
                break;
            case CONS:
                if (depth >0)
                    printf("<#LIST: ");
                lisp_print_list_aux(e, depth + 1);
                printf(">");
                break;
            case STRING:
                printf("<#STRING: \"%s\">", e->string);
                break;
            case FIXNUM:
                printf("<#FIXNUM: %li>", fixnum(e));
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
            case FN:
                printf("<#FN: %li>", (long)e);
                break;
        }
        ptr = rest(ptr);
    }
    if (listp(i)) {
        sep();
        printf(")");
    }
}


