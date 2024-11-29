#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define F_UPPERCASE (1 << 0)

int add_or_error(ssize_t r, int b) {
    if (r < 0) {
        return -1;
    } else if (r + b < b) {
        errno = EOVERFLOW;
        return -1;
    }
    return r + b;
}

char digit_to_char(unsigned int d, int flags) {
    if (d < 10) {
        return d + '0';
    } else {
        if (flags & F_UPPERCASE) {
            return d - 10 + 'A';
        } else {
            return d - 10 + 'a';
        }
    }
}

int print_signed(long long n, int base, int flags) {
    char c = digit_to_char(n % base, flags);
    int b = 0;
    if (n < 0) {
        n = -n;
        c = '-';
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
        if ((b = add_or_error(print_signed(n, base, flags), b)) < 0) {
            return -1;
        }
        return b;
    } else if (n >= base) {
        if ((b = add_or_error(print_signed(n / base, base, flags), b)) < 0) {
            return -1;
        }
    }
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }
    return b;
}

int print_unsigned(unsigned long long n, unsigned int base, int flags) {
    char c = digit_to_char(n % base, flags);
    int b = 0;
    if (n >= base) {
        if ((b = print_unsigned(n / base, base, flags)) < 0) {
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
    if ((b = add_or_error(print_unsigned((size_t)p, 16, 0), b)) < 0) {
        return -1;
    }
    return b;
}

int print_double(double n, int flags) {
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
        char nan[3];
        if (flags & F_UPPERCASE) {
            nan[0] = 'N';
            nan[1] = 'A';
            nan[2] = 'N';
        } else {
            nan[0] = 'n';
            nan[1] = 'a';
            nan[2] = 'n';
        }
        if ((b = add_or_error(write(STDOUT_FILENO, nan, 3), b)) < 0) {
            return -1;
        }
        return b;
    }
    if (isinf(n)) {
        char inf[3];
        if (flags & F_UPPERCASE) {
            inf[0] = 'I';
            inf[1] = 'N';
            inf[2] = 'F';
        } else {
            inf[0] = 'i';
            inf[1] = 'n';
            inf[2] = 'f';
        }
        if ((b = add_or_error(write(STDOUT_FILENO, inf, 3), b)) < 0) {
            return -1;
        }
        return b;
    }
    if ((b = add_or_error(print_unsigned((unsigned int)n, 10, flags), b)) < 0) {
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
    int flags = 0;
    if ((*fmt)[1] == 'c') {
        char c = va_arg(*args, int);
        *fmt += 2;
        return write(STDOUT_FILENO, &c, 1);
    } else if ((*fmt)[1] == 'd' || (*fmt)[1] == 'i') {
        int i = va_arg(*args, int);
        *fmt += 2;
        return print_signed(i, 10, flags);
    } else if ((*fmt)[1] == 'u') {
        unsigned int i = va_arg(*args, unsigned int);
        *fmt += 2;
        return print_unsigned(i, 10, flags);
    } else if ((*fmt)[1] == 'o') {
        int i = va_arg(*args, int);
        *fmt += 2;
        return print_unsigned(i, 8, flags);
    } else if ((*fmt)[1] == 'x') {
        int i = va_arg(*args, int);
        *fmt += 2;
        return print_unsigned(i, 16, flags);
    } else if ((*fmt)[1] == 'X') {
        int i = va_arg(*args, int);
        flags |= F_UPPERCASE;
        *fmt += 2;
        return print_unsigned(i, 16, flags);
    } else if ((*fmt)[1] == 'p') {
        void* p = va_arg(*args, void*);
        *fmt += 2;
        return print_pointer(p);
    } else if ((*fmt)[1] == 'f') {
        double d = va_arg(*args, double);
        *fmt += 2;
        return print_double(d, flags);
    } else if ((*fmt)[1] == 'F') {
        double d = va_arg(*args, double);
        flags |= F_UPPERCASE;
        *fmt += 2;
        return print_double(d, flags);
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
