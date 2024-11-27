#include <stdarg.h>
#include <unistd.h>

void print_num(int n) {
    char c = (n % 10) + '0';
    if (n < 0) {
        n = -n;
        c = '-';
        write(STDOUT_FILENO, &c, 1);
        print_num(n);
    } else if (n < 10) {
        write(STDOUT_FILENO, &c, 1);
    } else {
        print_num(n / 10);
        write(STDOUT_FILENO, &c, 1);
    }
}

void arg_parse(const char* restrict* fmt, va_list* args) {
    if ((*fmt)[1] == 'c') {
        char c = va_arg(*args, int);
        write(STDOUT_FILENO, &c, 1);
        *fmt += 2;
    } else if ((*fmt)[1] == 'd') {
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
