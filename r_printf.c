#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

int add_or_error(ssize_t r, int b) {
    if (r < 0) {
        return -1;
    } else if (r + b < b) {
        errno = EOVERFLOW;
        return -1;
    }
    return r + b;
}

char digit_to_char(unsigned int d) {
    if (d < 10) {
        return d + '0';
    } else {
        return d - 10 + 'a';
    }
}

int print_int(int n, int base) {
    char c = digit_to_char(n % base);
    int b = 0;
    if (n < 0) {
        n = -n;
        c = '-';
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
        if ((b = add_or_error(print_int(n, base), b)) < 0) {
            return -1;
        }
        return b;
    } else if (n >= base) {
        if ((b = add_or_error(print_int(n / base, base), b)) < 0) {
            return -1;
        }
    }
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }
    return b;
}

int print_uint(unsigned int n, unsigned int base) {
    char c = digit_to_char(n % base);
    int b = 0;
    if (n >= base) {
        if ((b = print_uint(n / base, base)) < 0) {
            return -1;
        }
    }
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }
    return b;
}

int print_pointer(void* p) {
    if (p == NULL) {
        char nil[] = "(nil)";
        return write(STDOUT_FILENO, nil, 5);
    }
    char prefix[] = "0x";
    int b = 0;
    if ((b = add_or_error(write(STDOUT_FILENO, prefix, 2), b)) < 0) {
        return -1;
    }
    if ((b = add_or_error(print_uint((size_t)p, 16), b)) < 0) {
        return -1;
    }
    return b;
}

int print_double(double n) {
    char c;
    int b = 0;
    if (signbit(n)) {
        n = -n;
        c = '-';
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    }
    if (isnan(n)) {
        char nan[] = "nan";
        if ((b = add_or_error(write(STDOUT_FILENO, nan, 3), b)) < 0) {
            return -1;
        }
        return b;
    }
    if (isinf(n)) {
        char inf[] = "inf";
        if ((b = add_or_error(write(STDOUT_FILENO, inf, 3), b)) < 0) {
            return -1;
        }
        return b;
    }
    if ((b = add_or_error(print_uint((unsigned int)n, 10), b)) < 0) {
        return -1;
    }
    c = '.';
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }
    int d = (n - (int)n) * 10;
    for (int i = 0; i < 6; i++) {
        c = d % 10 + '0';
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
        d *= 10;
    }
    return b;
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
    } else if ((*fmt)[1] == 'o') {
        int i = va_arg(*args, int);
        *fmt += 2;
        return print_uint(i, 8);
    } else if ((*fmt)[1] == 'x') {
        int i = va_arg(*args, int);
        *fmt += 2;
        return print_uint(i, 16);
    } else if ((*fmt)[1] == 'p') {
        void* p = va_arg(*args, void*);
        *fmt += 2;
        return print_pointer(p);
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
    int i = 0, b = 0;
    while (fmt[i] != '\0') {
        if (fmt[i] == '%') {
            // arg parse
            if ((b = add_or_error(write(STDOUT_FILENO, fmt, i), b)) < 0) {
                return -1;
            }
            fmt += i;
            if ((b = add_or_error(arg_parse(&fmt, &args), b)) < 0) {
                return -1;
            }
            i = 0;
        } else {
            i++;
        }
    }
    if ((b = add_or_error(write(STDOUT_FILENO, fmt, i), b)) < 0) {
        return -1;
    }
    va_end(args);
    return b;
}
