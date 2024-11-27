#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

// log base 10 of 2
#define LOG2 0.301029995664

void print_num(int n) {
    int i = 0;
    char s[(size_t)(8 * sizeof(int) * LOG2 + 2)];
    if (n < 0) {
        n = -n;
        s[i] = '-';
        i++;
    }
    int p = 1;
    for (int m = n; m > 9; m /= 10) {
        p *= 10;
    }

    for (int q = p; q > 0; q /= 10) {
        s[i] = (n / q) % 10 + '0';
        i++;
    }
    write(STDOUT_FILENO, s, i);
}

void arg_parse(const char* restrict* fmt, va_list* args) {
    if ((*fmt)[1] == 'c') {
        char c = va_arg(*args, int);
        write(STDOUT_FILENO, &c, 1);
        *fmt += 2;
    }

    if ((*fmt)[1] == 'd') {
        int i = va_arg(*args, int);
        print_num(i);
        *fmt += 2;
    }
}

int r_printf(const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int i = 0;
    while (fmt[i] != '\0') {
        if (fmt[i] == '%') {
            // arg parse
            write(STDOUT_FILENO, fmt, i);
            fmt += i;
            arg_parse(&fmt, &args);
            i = 0;
        } else {
            i++;
        }
    }
    write(STDOUT_FILENO, fmt, i);
    va_end(args);
    return 0;
}
