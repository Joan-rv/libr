#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define F_UPPERCASE (1 << 0)
#define F_SIGNALWAYS (1 << 1)
#define F_SPACE (1 << 2)
#define F_ALTERNATE (1 << 3)
#define F_LEFTADJUST (1 << 4)

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

int print_spaces(int n) {
    char c = ' ';
    int b = 0;
    for (int i = 0; i < n; i++) {
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    }
    return b;
}

int print_unsigned(unsigned long long n, unsigned int base, int flags,
                   int width) {
    char c;
    int b = 0;
    int num_width = 0;
    if (flags & F_ALTERNATE) {
        if (base == 8) {
            num_width++;
        } else if (base == 16) {
            num_width += 2;
        }
    }
    num_width += (long long)(log2(n) / log2(base)) + 1;
    if (!(flags & F_LEFTADJUST)) {
        if ((b = add_or_error(print_spaces(width - num_width), b)) < 0) {
            return -1;
        }
    }
    if ((base == 8 || base == 16) && flags & F_ALTERNATE) {
        if (base == 8) {
            c = '0';
            if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
                return -1;
            }
        } else if (base == 16) {
            char s[2];
            if (flags & F_UPPERCASE) {
                memcpy(s, "0X", 2);
            } else {
                memcpy(s, "0x", 2);
            }

            if ((b = add_or_error(write(STDOUT_FILENO, s, 2), b)) < 0) {
                return -1;
            }
        }
    }
    c = digit_to_char(n % base, flags);
    if (n >= base) {
        if ((b = add_or_error(
                 print_unsigned(n / base, base, flags & ~F_ALTERNATE, 0), b)) <
            0) {
            return -1;
        }
    }
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }
    if (flags & F_LEFTADJUST) {
        if ((b = add_or_error(print_spaces(width - num_width), b)) < 0) {
            return -1;
        }
    }
    return b;
}

int print_signed(long long n, int base, int flags, int width) {
    char c;
    int b = 0;

    int num_width = 0;
    if (n < 0 || flags & (F_SIGNALWAYS | F_SPACE)) {
        num_width++;
    }
    if (n < 0) {
        num_width += (long long)(log2(-n) / log2(base)) + 1;
    } else {
        num_width += (long long)(log2(n) / log2(base)) + 1;
    }
    if (!(flags & F_LEFTADJUST)) {
        if ((b = add_or_error(print_spaces(width - num_width), b)) < 0) {
            return -1;
        }
    }

    if (n < 0) {
        n = -n;
        c = '-';
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    } else if (flags & (F_SIGNALWAYS | F_SPACE)) {
        if (flags & F_SIGNALWAYS) {
            c = '+';
        } else {
            c = ' ';
        }
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    }
    if ((b = add_or_error(print_unsigned(n, base, flags & ~F_SIGNALWAYS, 0),
                          b)) < 0) {
        return -1;
    }
    if (flags & F_LEFTADJUST) {
        if ((b = add_or_error(print_spaces(width - num_width), b)) < 0) {
            return -1;
        }
    }
    return b;
}

int print_pointer(void* p, int flags, int width) {
    int num_width = 0;
    int b = 0;
    if (p == NULL) {
        num_width = 5;
    } else {
        num_width = 2 + (long long)log2((size_t)p) / 4 + 1;
    }
    if (!(flags & F_LEFTADJUST)) {
        if ((b = add_or_error(print_spaces(width - num_width), b)) < 0) {
            return -1;
        }
    }
    if (p == NULL) {
        char nil[] = "(nil)";
        return add_or_error(write(STDOUT_FILENO, nil, 5), b);
    }
    char prefix[] = "0x";
    if ((b = add_or_error(write(STDOUT_FILENO, prefix, 2), b)) < 0) {
        return -1;
    }
    if ((b = add_or_error(print_unsigned((size_t)p, 16, 0, 0), b)) < 0) {
        return -1;
    }
    if (flags & F_LEFTADJUST) {
        if ((b = add_or_error(print_spaces(width - num_width), b)) < 0) {
            return -1;
        }
    }
    return b;
}

bool handle_nan_or_inf(double n, int flags, int* b) {
    if (isnan(n)) {
        char nan[3];
        if (flags & F_UPPERCASE) {
            memcpy(nan, "NAN", 3);
        } else {
            memcpy(nan, "nan", 3);
        }
        if ((*b = add_or_error(write(STDOUT_FILENO, nan, 3), *b)) < 0) {
            *b = -1;
        }
        return true;
    } else if (isinf(n)) {
        char inf[3];
        if (flags & F_UPPERCASE) {
            memcpy(inf, "INF", 3);
        } else {
            memcpy(inf, "inf", 3);
        }
        if ((*b = add_or_error(write(STDOUT_FILENO, inf, 3), *b)) < 0) {
            *b = -1;
        }
        return true;
    } else {
        return false;
    }
}

int print_decimal(double n, int flags) {
    char c;
    int b = 0;
    if (signbit(n)) {
        n = -n;
        c = '-';
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    } else if (flags & (F_SIGNALWAYS | F_SPACE)) {
        if (flags & F_SIGNALWAYS) {
            c = '+';
        } else {
            c = ' ';
        }
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    }
    if (handle_nan_or_inf(n, flags, &b)) {
        return b;
    }
    if ((b = add_or_error(print_unsigned((unsigned int)n, 10, flags, 0), b)) <
        0) {
        return -1;
    }
    c = '.';
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }
    // add 0.0000005 to round (6 zeros after decimal point)
    double d = (n + 0.0000005f - (int)n) * 10;
    for (int i = 0; i < 6; i++) {
        c = (int)d % 10 + '0';
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
        d *= 10;
    }
    return b;
}

int print_exponential(double n, int flags) {
    char c;
    int b = 0;
    if (signbit(n)) {
        n = -n;
        c = '-';
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    } else if (flags & (F_SIGNALWAYS | F_SPACE)) {
        if (flags & F_SIGNALWAYS) {
            c = '+';
        } else {
            c = ' ';
        }
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    }
    if (handle_nan_or_inf(n, flags, &b)) {
        return b;
    }
    int e = 6, m;
    if (n == 0) {
        m = 0;
        e = 0;
    } else {
        double g = n;
        m = n;
        while (m < 1000000) {
            g *= 10;
            m = g;
            e--;
        }
        while (m > 9999999) {
            g /= 10;
            m = g;
            e++;
        }
        // round m
        m = ((int)(g * 10) + 5) / 10;
    }
    c = digit_to_char((m / 1000000) % 10, flags);
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }
    c = '.';
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }
    for (int p = 100000; p > 0; p /= 10) {
        c = digit_to_char((m / p) % 10, flags);
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    }
    if (flags & F_UPPERCASE) {
        c = 'E';
    } else {
        c = 'e';
    }
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }

    if (e > 9) {
        print_signed(e, 10, 0, 0);
    } else {
        if (e < 0) {
            c = '-';
            e = -e;
        } else {
            c = '+';
        }
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
        for (int p = 10; p > 0; p /= 10) {
            c = digit_to_char((e / p) % 10, 0);
            if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
                return -1;
            }
        }
    }

    return b;
}

int arg_parse(const char* restrict* fmt, va_list* args, int flags) {
    // read flags
    switch ((*fmt)[1]) {
    case ' ':
        flags |= F_SPACE;
        *fmt += 1;
        return arg_parse(fmt, args, flags);
    case '+':
        flags |= F_SIGNALWAYS;
        *fmt += 1;
        return arg_parse(fmt, args, flags);
    case '-':
        flags |= F_LEFTADJUST;
        *fmt += 1;
        return arg_parse(fmt, args, flags);
    case '#':
        // TODO: handle floating point alternate formats
        flags |= F_ALTERNATE;
        *fmt += 1;
        return arg_parse(fmt, args, flags);
    }

    // read width
    int width = 0;
    while ('0' <= (*fmt)[1] && (*fmt)[1] <= '9') {
        width = width * 10 + (*fmt)[1] - '0';
        *fmt += 1;
    }

    // read precision
    if ((*fmt)[1] == '.') {
        *fmt += 1;
        int precision = 0;
        while ('0' <= (*fmt)[1] && (*fmt)[1] <= '9') {
            precision = precision * 10 + (*fmt)[1] - '0';
            *fmt += 1;
        }
    }

    // read conversion
    switch ((*fmt)[1]) {
    case 'c': {
        char c = va_arg(*args, int);
        *fmt += 2;
        return write(STDOUT_FILENO, &c, 1);
    }
    case 'd':
    case 'i': {
        int i = va_arg(*args, int);
        *fmt += 2;
        return print_signed(i, 10, flags, width);
    }
    case 'u': {
        unsigned int i = va_arg(*args, unsigned int);
        *fmt += 2;
        return print_unsigned(i, 10, flags, width);
    }
    case 'o': {
        unsigned int i = va_arg(*args, int);
        *fmt += 2;
        return print_unsigned(i, 8, flags, width);
    }
    case 'x': {
        unsigned int i = va_arg(*args, int);
        *fmt += 2;
        return print_unsigned(i, 16, flags, width);
    }
    case 'X': {
        unsigned int i = va_arg(*args, int);
        flags |= F_UPPERCASE;
        *fmt += 2;
        return print_unsigned(i, 16, flags, width);
    }
    case 'p': {
        void* p = va_arg(*args, void*);
        *fmt += 2;
        return print_pointer(p, flags, width);
    }
    case 'f': {
        double d = va_arg(*args, double);
        *fmt += 2;
        return print_decimal(d, flags);
    }
    case 'F': {
        double d = va_arg(*args, double);
        flags |= F_UPPERCASE;
        *fmt += 2;
        return print_decimal(d, flags);
    }
    case 'e': {
        double d = va_arg(*args, double);
        *fmt += 2;
        return print_exponential(d, flags);
    }
    case 'E': {
        double d = va_arg(*args, double);
        flags |= F_UPPERCASE;
        *fmt += 2;
        return print_exponential(d, flags);
    }
    case 's': {
        char* s = va_arg(*args, char*);
        *fmt += 2;
        int n = strlen(s);
        return write(STDOUT_FILENO, s, n);
    }
    case 'm': {
        char* s = strerror(errno);
        int n = strlen(s);
        *fmt += 2;
        return write(STDOUT_FILENO, s, n);
    }
    case '%': {
        *fmt += 2;
        return write(STDOUT_FILENO, *fmt - 2, 1);
    }
    default: {
        return -1;
    }
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
            if ((b = add_or_error(arg_parse(&fmt, &args, 0), b)) < 0) {
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
