#include "lisp_mu.h"

int main() {
    printf("Hello, World!\n");
    printf("Sum of 15 and 56 = %d\n",  sum_int(2, 15, 56) );
#ifdef WITH_FLOATING_POINT
    printf("Sum of 15.5 and 56.6 = %f\n",  sum_double(2, 15.5, 56.6) );
#endif

    lisp_init();
    cell result;
    // REPL
    do {
//        cell input = lisp_read();
//        result = lisp_eval(input);
        lisp_pprint(result, 0);
    } while(result != nil);

    return 0;
}
