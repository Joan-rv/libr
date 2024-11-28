#include <stdarg.h>
#include <string.h>
#include <unistd.h>

char digit_to_char(unsigned int d) {
    if (d < 10) {
        return d + '0';
    } else {
        return d + 'a';
    }
}

int print_int(int n, unsigned int b) {
    char c = digit_to_char(n % b);
    int m = 0;
    if (n < 0) {
        n = -n;
        c = '-';
        if (write(STDOUT_FILENO, &c, 1) < 0) {
            return -1;
        }
        m = print_int(n, b);
        return 1 + m;
    } else if (n > 9) {
        m = print_int(n / 10, 10);
    }
    if (write(STDOUT_FILENO, &c, 1) < 0) {
        return -1;
    }
    return 1 + m;
}

int print_uint(unsigned int n, unsigned int b) {
    char c = digit_to_char(n % b);
    int m = 0;
    if (n > 9) {
        m = print_uint(n / 10, 10);
    }
    if (write(STDOUT_FILENO, &c, 1) < 0) {
        return -1;
    }
    return 1 + m;
}

int print_double(double n) {
    int m = print_int((int)n, 10);
    if (n < 0) {
        n = -n;
    }
    char c = '.';
    if (write(STDOUT_FILENO, &c, 1) < 0) {
        return -1;
    }
    int d = (n - (int)n) * 10;
    for (int i = 0; i < 6; i++) {
        c = d % 10 + '0';
        if (write(STDOUT_FILENO, &c, 1) < 0) {
            return -1;
        }
        d *= 10;
    }
    return m + 7;
}

int arg_parse(const char* restrict* fmt, va_list* args) {
    if ((*fmt)[1] == 'c') {
        char c = va_arg(*args, int);
        *fmt += 2;
        return write(STDOUT_FILENO, &c, 1);
    } else if ((*fmt)[1] == 'd' || (*fmt)[1] == 'i') {
        int i = va_arg(*args, int);
        *fmt += 2;
        return print_int(i, 10);
    } else if ((*fmt)[1] == 'u') {
        unsigned int i = va_arg(*args, unsigned int);
        *fmt += 2;
        return print_uint(i, 10);
    } else if ((*fmt)[1] == 'f') {
        double d = va_arg(*args, double);
        *fmt += 2;
        return print_double(d);
    } else if ((*fmt)[1] == 's') {
        char* s = va_arg(*args, char*);
        *fmt += 2;
        int n = strlen(s);
        return write(STDOUT_FILENO, s, n);
    } else if ((*fmt)[1] == '%') {
        *fmt += 2;
        return write(STDOUT_FILENO, *fmt - 2, 1);
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
            if (write(STDOUT_FILENO, fmt, i) == -1) {
                return -1;
            }
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
    if (write(STDOUT_FILENO, fmt, i) < 0) {
        return -1;
    }
    n += i;
    va_end(args);
    return n;
}
