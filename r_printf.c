#include <stdarg.h>
#include <unistd.h>

int print_num(int n) {
    char c = (n % 10) + '0';
    if (n < 0) {
        n = -n;
        c = '-';
        write(STDOUT_FILENO, &c, 1);
        return 1 + print_num(n);
    } else if (n < 10) {
        write(STDOUT_FILENO, &c, 1);
        return 1;
    } else {
        int m = print_num(n / 10);
        write(STDOUT_FILENO, &c, 1);
        return 1 + m;
    }
}

int arg_parse(const char* restrict* fmt, va_list* args) {
    if ((*fmt)[1] == 'c') {
        char c = va_arg(*args, int);
        *fmt += 2;
        write(STDOUT_FILENO, &c, 1);
        return 1;
    } else if ((*fmt)[1] == 'd') {
        int i = va_arg(*args, int);
        *fmt += 2;
        return print_num(i);
    } else if ((*fmt)[1] == '%') {
        write(STDOUT_FILENO, *fmt, 1);
        *fmt += 2;
        return 1;
    } else {
        return -1;
    }
}

int r_printf(const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int i = 0, n = 0;
    while (fmt[i] != '\0') {
        if (fmt[i] == '%') {
            // arg parse
            write(STDOUT_FILENO, fmt, i);
            n += i;
            fmt += i;
            int m;
            if ((m = arg_parse(&fmt, &args)) < 0) {
                return -1;
            } else {
                n += m;
            }
            i = 0;
        } else {
            i++;
        }
    }
    write(STDOUT_FILENO, fmt, i);
    n += i;
    va_end(args);
    return n;
}
