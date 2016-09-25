#include "lisp_mu.h"
#include <stdio.h>
#include <stdlib.h>
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
#define toggle_quoted (in_quotes = !in_quotes)
#define toggle_escape (escaped = !escaped)

#define BUF_SIZE 1024

int main() {
    init_printf(NULL, my_putc);
    lisp_init();
    cell exp, result;
    char *buf;
    int i = 0;
    char c = 0;

    buf = malloc(sizeof(char) * BUF_SIZE);
    memset(buf, 0, BUF_SIZE);

    while (1) {
        c = getc(stdin);
        if (escaped) {
            toggle_escape;
            buf[i++] = c;
        } else if (c == '\\' && in_quotes) {
            toggle_escape;
            buf[i++] = c;
        } else if (c == '"') {
            toggle_quoted;
            buf[i++] = c;
        } else if (c == '(' && !in_quotes) {
            incr;
            buf[i++] = c;
        } else if (c == ')' && !in_quotes) {
            decr;
            buf[i++] = c;
        } else {
            buf[i++] = c;
        }

        if (brackets_are_balanced && c == 10) {
            buf[i++] = '\0';
            puts("Execute now!");
            exp = lisp_read((const char **) &buf);
            result = eval(exp, global_env);
            lisp_pprint(result);
            memset(buf, 0, BUF_SIZE);
            i = 0;
        }

        if (i >= BUF_SIZE) {
            printf("Buffer is too large (max = %i)\r\n", BUF_SIZE);
            memset(buf, 0, BUF_SIZE);
            i = 0;
        }

        //printf("depth: %i \r\n", brackets);
    }
    lisp_cleanup();
}

