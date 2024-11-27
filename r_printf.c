#include <stdarg.h>
#include <unistd.h>

int print_int(int n) {
    char c = (n % 10) + '0';
    if (n < 0) {
        n = -n;
        c = '-';
        write(STDOUT_FILENO, &c, 1);
        return 1 + print_int(n);
    } else if (n < 10) {
        write(STDOUT_FILENO, &c, 1);
        return 1;
    } else {
        int m = print_int(n / 10);
        write(STDOUT_FILENO, &c, 1);
        return 1 + m;
    }
}

int print_double(double n) {
    print_int((int)n);
    if (n < 0) {
        n = -n;
    }
    char c = '.';
    int m = write(STDOUT_FILENO, &c, 1);
    int d = (n - (int)n) * 10;
    for (int i = 0; i < 6; i++) {
        c = d % 10 + '0';
        write(STDOUT_FILENO, &c, 1);
        d *= 10;
    }
    return m + 7;
}

int arg_parse(const char* restrict* fmt, va_list* args) {
    if ((*fmt)[1] == 'c') {
        char c = va_arg(*args, int);
        *fmt += 2;
        write(STDOUT_FILENO, &c, 1);
        return 1;
    } else if ((*fmt)[1] == 'd' || (*fmt)[1] == 'i') {
        int i = va_arg(*args, int);
        *fmt += 2;
        return print_int(i);
    } else if ((*fmt)[1] == 'f') {
        double d = va_arg(*args, double);
        *fmt += 2;
        return print_double(d);
    } else if ((*fmt)[1] == '%') {
        write(STDOUT_FILENO, *fmt, 1);
        *fmt += 2;
        return 1;
    } else {
        return -1;
    }
}

// TODO: proper error handling:
// 1. Check for write() errors (man 2 write)
// 2. Set errno on error (man 3p fprintf)
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
