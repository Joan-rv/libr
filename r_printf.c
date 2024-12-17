#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef enum {
    F_UPPERCASE = 1 << 0,
    F_SIGNALWAYS = 1 << 1,
    F_SPACE = 1 << 2,
    F_ALTERNATE = 1 << 3,
    F_LEFTADJUST = 1 << 4,
    F_ZEROPAD = 1 << 5,
} Flags;

typedef enum {
    L_CHAR = 1 << 0,
    L_SHORT = 1 << 1,
    L_LONG = 1 << 2,
    L_LONGLONG = 1 << 3,
    L_LONGDOUBLE = 1 << 4,
    L_INTMAX = 1 << 5,
    L_SIZET = 1 << 6,
    L_PTRDIFF = 1 << 7,
} Length;

int add_or_error(ssize_t r, int b) {
    if (r < 0) {
        return -1;
    } else if (r + b < b) {
        errno = EOVERFLOW;
        return -1;
    }
    return r + b;
}

char digit_to_char(unsigned int d, Flags flags) {
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

int print_padding(int n) {
    char c = ' ';
    int b = 0;
    for (int i = 0; i < n; i++) {
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }
    }
    return b;
}

int print_unsigned(uintmax_t n, unsigned int base, Flags flags, int width,
                   int precision) {
    char c;
    int b = 0;
    int num_width;
    int num_digits;
    if (precision > 0 || width != 0) {
        num_digits = (long long)(log2(n) / log2(base)) + 1;
    }
    if (width != 0) {
        num_width = 0;
        if (flags & F_ALTERNATE) {
            if (base == 8) {
                num_width++;
            } else if (base == 16) {
                num_width += 2;
            }
        }
        if (precision > num_digits) {
            num_width += precision;
        } else {
            num_width += num_digits;
        }
    }

    if (!(flags & F_LEFTADJUST) && flags & F_ZEROPAD && precision == -1) {
        precision = width;
        if (flags & F_ALTERNATE) {
            if (base == 8) {
                precision--;
            } else if (base == 16) {
                precision -= 2;
            }
        }
        width = 0;
    }

    if (!(flags & F_LEFTADJUST) && width != 0) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
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

    if (precision > 0) {
        c = '0';
        for (int i = 0; i < precision - num_digits; i++) {
            if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
                return -1;
            }
        }
    }

    c = digit_to_char(n % base, flags);
    if (n >= base) {
        if ((b = add_or_error(
                 print_unsigned(n / base, base, flags & ~F_ALTERNATE, 0, 0),
                 b)) < 0) {
            return -1;
        }
    }
    if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
        return -1;
    }
    if (flags & F_LEFTADJUST && width != 0) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
            return -1;
        }
    }
    return b;
}

int print_signed(intmax_t n, int base, Flags flags, int width, int precision) {
    char c;
    int b = 0;

    int num_width = 0;
    if (n < 0 || flags & (F_SIGNALWAYS | F_SPACE)) {
        num_width++;
    }
    int num_digits;
    if (n < 0) {
        num_digits = (long long)(log2(-n) / log2(base)) + 1;
    } else {
        num_digits = (long long)(log2(n) / log2(base)) + 1;
    }
    if (precision > num_digits) {
        num_width += precision;
    } else {
        num_width += num_digits;
    }
    if (!(flags & F_LEFTADJUST) && flags & F_ZEROPAD && precision == -1) {
        precision = width;
        if (n < 0 || flags & (F_SIGNALWAYS | F_SPACE)) {
            precision--;
        }
        width = 0;
    }

    if (!(flags & F_LEFTADJUST)) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
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
    if ((b = add_or_error(
             print_unsigned(n, base, flags & ~F_SIGNALWAYS, 0, precision), b)) <
        0) {
        return -1;
    }
    if (flags & F_LEFTADJUST) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
            return -1;
        }
    }
    return b;
}

int print_pointer(void* p, Flags flags, int width) {
    int num_width = 0;
    int b = 0;
    if (p == NULL) {
        num_width = 5;
    } else {
        num_width = 2 + (long long)log2((size_t)p) / 4 + 1;
    }
    if (!(flags & F_LEFTADJUST)) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
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
    if ((b = add_or_error(print_unsigned((size_t)p, 16, 0, 0, 0), b)) < 0) {
        return -1;
    }
    if (flags & F_LEFTADJUST) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
            return -1;
        }
    }
    return b;
}

bool handle_nan_or_inf(double n, Flags flags, int* b) {
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

int print_decimal(double n, int base, Flags flags, int width, int precision) {
    char c;
    int b = 0;
    int num_width = 0;
    int zeropad_precision;
    if (signbit(n) || flags & (F_SIGNALWAYS | F_SPACE)) {
        num_width++;
    }
    if (isnan(n) || isinf(n)) {
        num_width += 3;
    } else {
        if (signbit(n)) {
            zeropad_precision = (long long)(log10(-n)) + 1;
        } else {
            zeropad_precision = (long long)(log10(n)) + 1;
        }
        num_width += zeropad_precision;
    }
    if (precision > 0) {
        num_width += precision + 1;
    } else if (flags & F_ALTERNATE) {
        num_width++;
    }
    if (precision < 0) {
        precision = 6;
    }

    if (!(flags & F_LEFTADJUST) && flags & F_ZEROPAD) {
        zeropad_precision += width - num_width;
        width = 0;
    } else {
        zeropad_precision = 0;
    }

    if (!(flags & F_LEFTADJUST)) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
            return -1;
        }
    }
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
    if (!handle_nan_or_inf(n, flags, &b)) {
        // add 5 to last decimal to round up
        n += 0.5f * pow(base, -precision);
        if ((b = add_or_error(print_unsigned((unsigned int)n, base,
                                             flags | F_ALTERNATE, 0,
                                             zeropad_precision),
                              b)) < 0) {
            return -1;
        }
        if (precision > 0 || flags & F_ALTERNATE) {
            c = '.';
            if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
                return -1;
            }
        }
        if (precision > 0) {
            double d = (n - (int)n) * base;
            for (int i = 0; i < precision; i++) {
                c = digit_to_char((int)d % base, flags);
                if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
                    return -1;
                }
                d *= base;
            }
        }
    }
    if (flags & F_LEFTADJUST) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
            return -1;
        }
    }
    return b;
}

int print_exponential(double n, int base, Flags flags, int width,
                      int precision) {
    char c;
    int b = 0;

    int e = 0;
    double m = n;
    if (m != 0) {
        while (m < 1) {
            m *= base;
            e--;
        }
        while (m >= base) {
            m /= base;
            e++;
        }
    }

    int exponent_precision = 2;
    if (base == 2) {
        base = 16;
        exponent_precision = 1;
    }

    int zeropad_precision;
    int num_width = 0;
    if (signbit(n) || flags & (F_SIGNALWAYS | F_SPACE)) {
        num_width++;
    }
    if (isnan(n) || isinf(n)) {
        num_width += 3;
    } else if (e < 100) {
        if (base == 10 || e >= 10) {
            zeropad_precision = 2;
            num_width += 2;
        } else {
            zeropad_precision = 1;
            num_width += 1;
        }
    } else {
        if (e < 0) {
            zeropad_precision = (long long)(log(-n) / log(base)) + 1;
        } else {
            zeropad_precision = (long long)(log(n) / log(base)) + 1;
        }
        num_width += zeropad_precision;
    }

    if (precision < 0) {
        precision = 6;
    }
    if (base == 16) {
        num_width += 2;
        zeropad_precision += 2;
    }
    if (e < 0) {
        num_width++;
    }
    num_width += precision + 3;

    if (!isnan(n) && !isinf(n) && !(flags & F_LEFTADJUST) &&
        flags & F_ZEROPAD) {
        zeropad_precision = width - zeropad_precision - 2;
        width = 0;
    } else {
        zeropad_precision = 0;
    }

    if (!(flags & F_LEFTADJUST)) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
            return -1;
        }
    }
    if (!handle_nan_or_inf(n, flags, &b)) {
        if ((b = add_or_error(
                 print_decimal(m, base, flags, zeropad_precision, precision),
                 b)) < 0) {
            return -1;
        }
        if (flags & F_UPPERCASE) {
            if (base == 16) {
                c = 'P';
            } else {
                c = 'E';
            }
        } else {
            if (base == 16) {
                c = 'p';
            } else {
                c = 'e';
            }
        }
        if ((b = add_or_error(write(STDOUT_FILENO, &c, 1), b)) < 0) {
            return -1;
        }

        if ((b = add_or_error(print_signed(e, base, 0, 0, exponent_precision),
                              b)) < 0) {
            return -1;
        }
    }

    if (flags & F_LEFTADJUST) {
        if ((b = add_or_error(print_padding(width - num_width), b)) < 0) {
            return -1;
        }
    }
    return b;
}

Length read_length_modifier(const char* restrict* fmt) {
    Length length = 0;
    while (true) {
        switch ((*fmt)[1]) {
        case 'h':
            if (length & L_SHORT) {
                length |= L_CHAR;
            } else {
                length |= L_SHORT;
            }
            break;
        case 'l':
            if (length & L_LONG) {
                length |= L_LONGLONG;
            } else {
                length |= L_LONGLONG;
            }
            break;
        case 'q':
            length |= L_LONGLONG;
            break;
        case 'L':
            length |= L_LONGDOUBLE;
            break;
        case 'j':
            length |= L_INTMAX;
            break;
        case 'Z':
        case 'z':
            length |= L_SIZET;
            break;
        case 't':
            length |= L_PTRDIFF;
            break;
        default:
            return length;
        }
        (*fmt)++;
    }
}

intmax_t read_signed(va_list* args, Length length) {
    if (length & L_CHAR) {
        return (char)va_arg(*args, int);
    } else if (length & L_SHORT) {
        return (short)va_arg(*args, int);
    } else if (length & L_LONG) {
        return (long)va_arg(*args, long);
    } else if (length & L_LONGLONG) {
        return (long long)va_arg(*args, long long);
    } else if (length & L_INTMAX) {
        return va_arg(*args, intmax_t);
    } else if (length & L_SIZET) {
        return (ssize_t)va_arg(*args, ssize_t);
    } else if (length & L_PTRDIFF) {
        return (ptrdiff_t)va_arg(*args, ptrdiff_t);
    } else {
        return (int)va_arg(*args, int);
    }
}

uintmax_t read_unsigned(va_list* args, Length length) {
    if (length & L_CHAR) {
        return (unsigned char)va_arg(*args, unsigned int);
    } else if (length & L_SHORT) {
        return (unsigned short)va_arg(*args, unsigned int);
    } else if (length & L_LONGLONG) {
        return (unsigned long long)va_arg(*args, unsigned long long);
    } else if (length & L_INTMAX) {
        return va_arg(*args, uintmax_t);
    } else if (length & L_SIZET) {
        return (size_t)va_arg(*args, size_t);
    } else if (length & L_PTRDIFF) {
        return (ptrdiff_t)va_arg(*args, ptrdiff_t);
    } else {
        return (unsigned int)va_arg(*args, int);
    }
}

int arg_parse(const char* restrict* fmt, va_list* args, Flags flags) {
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
    case '0':
        flags |= F_ZEROPAD;
        *fmt += 1;
        return arg_parse(fmt, args, flags);
    case '#':
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
    int precision = -1;
    if ((*fmt)[1] == '.') {
        precision = 0;
        *fmt += 1;
        while ('0' <= (*fmt)[1] && (*fmt)[1] <= '9') {
            precision = precision * 10 + (*fmt)[1] - '0';
            *fmt += 1;
        }
    }

    Length length = read_length_modifier(fmt);

    // read conversion
    switch ((*fmt)[1]) {
    case 'c': {
        char c = va_arg(*args, int);
        *fmt += 2;
        return write(STDOUT_FILENO, &c, 1);
    }
    case 'd':
    case 'i': {
        intmax_t i = read_signed(args, length);
        *fmt += 2;
        return print_signed(i, 10, flags, width, precision);
    }
    case 'u': {
        uintmax_t i = read_unsigned(args, length);
        *fmt += 2;
        return print_unsigned(i, 10, flags, width, precision);
    }
    case 'o': {
        uintmax_t i = read_unsigned(args, length);
        *fmt += 2;
        return print_unsigned(i, 8, flags, width, precision);
    }
    case 'x': {
        uintmax_t i = read_unsigned(args, length);
        *fmt += 2;
        return print_unsigned(i, 16, flags, width, precision);
    }
    case 'X': {
        uintmax_t i = read_unsigned(args, length);
        flags |= F_UPPERCASE;
        *fmt += 2;
        return print_unsigned(i, 16, flags, width, precision);
    }
    case 'p': {
        void* p = va_arg(*args, void*);
        *fmt += 2;
        return print_pointer(p, flags, width);
    }
    case 'f': {
        double d = va_arg(*args, double);
        *fmt += 2;
        return print_decimal(d, 10, flags, width, precision);
    }
    case 'F': {
        double d = va_arg(*args, double);
        flags |= F_UPPERCASE;
        *fmt += 2;
        return print_decimal(d, 10, flags, width, precision);
    }
    case 'e': {
        double d = va_arg(*args, double);
        *fmt += 2;
        return print_exponential(d, 10, flags, width, precision);
    }
    case 'E': {
        double d = va_arg(*args, double);
        flags |= F_UPPERCASE;
        *fmt += 2;
        return print_exponential(d, 10, flags, width, precision);
    }
    case 'a': {
        double d = va_arg(*args, double);
        *fmt += 2;
        return print_exponential(d, 2, flags, width, precision);
    }
    case 'A': {
        double d = va_arg(*args, double);
        flags |= F_UPPERCASE;
        *fmt += 2;
        return print_exponential(d, 2, flags, width, precision);
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
