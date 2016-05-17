#include "lisp_mu.h"
#include <stdio.h>
#include "tinyprintf.h"

void my_putc( void* p, char c) {
    putc(c, stdout);
    fflush(stdout);
}

int brackets = 0;
bool in_quotes = false;
bool escaped = false;
#define incr  (++brackets)
#define decr  (--brackets)
#define brackets_are_balanced (brackets == 0)
#define toggle_quoted() (in_quotes = !in_quotes)
#define toggle_escape() (escaped = !escaped)

int main() {
    init_printf(NULL, my_putc);
    for (char c=0; ; c = getc(stdin)) {
        if (escaped) {
            toggle_escape;
            // store the value
        } else if (c == '\\' && in_quotes)
            toggle_escape;
        else if (c == '"')
            toggle_quoted;
        else if (c == '(' && !in_quotes)
            incr;
        else if (c == ')' && !in_quotes)
            decr;

        if (brackets_are_balanced && c == 10) puts("Execute now!");

        printf("depth: %i \r\n", brackets);
    }


    return 0;
}

