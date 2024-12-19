#include <errno.h>
#include <r_printf.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

int add_or_error(ssize_t r, int b) {
    if (r < 0) {
        return -1;
    } else if (r + b < b) {
        errno = EOVERFLOW;
        return -1;
    }
    return r + b;
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
                length |= L_LONG;
            }
            break;
        case 'q':
        case 'L':
            length |= L_LONGLONG;
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

long double read_double(va_list* args, Length length) {
    if (length & L_LONGLONG) {
        return va_arg(*args, long double);
    } else {
        return (double)va_arg(*args, double);
    }
}

size_t read_char(va_list* args, Length length, char out[]) {
    if (length & L_LONG) {
        wint_t c = va_arg(*args, wint_t);
        mbstate_t state;
        memset(&state, 0, sizeof(state));
        return wcrtomb(out, c, &state);
    } else {
        out[0] = (char)va_arg(*args, int);
        return 1;
    }
}

char* read_string(va_list* args, Length length) {
    if (length & L_LONG) {
        wchar_t* ws = va_arg(*args, wchar_t*);
        size_t l = wcstombs(NULL, ws, 0);
        if (l == (size_t)-1) {
            return NULL;
        }
        char* s = malloc(l + 1);
        if (s == NULL) {
            return NULL;
        }
        wcstombs(s, ws, l + 1);
        return s;
    } else {
        return (char*)va_arg(*args, char*);
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
    *fmt += 2;
    switch ((*fmt)[-1]) {
    case 'd':
    case 'i': {
        intmax_t i = read_signed(args, length);
        return print_signed(i, 10, flags, width, precision);
    }
    case 'u': {
        uintmax_t i = read_unsigned(args, length);
        return print_unsigned(i, 10, flags, width, precision);
    }
    case 'o': {
        uintmax_t i = read_unsigned(args, length);
        return print_unsigned(i, 8, flags, width, precision);
    }
    case 'x': {
        uintmax_t i = read_unsigned(args, length);
        return print_unsigned(i, 16, flags, width, precision);
    }
    case 'X': {
        uintmax_t i = read_unsigned(args, length);
        flags |= F_UPPERCASE;
        return print_unsigned(i, 16, flags, width, precision);
    }
    case 'p': {
        void* p = va_arg(*args, void*);
        return print_pointer(p, flags, width);
    }
    case 'f': {
        long double d = read_double(args, length);
        return print_decimal(d, 10, flags, width, precision);
    }
    case 'F': {
        long double d = read_double(args, length);
        flags |= F_UPPERCASE;
        return print_decimal(d, 10, flags, width, precision);
    }
    case 'e': {
        long double d = read_double(args, length);
        return print_exponential(d, 10, flags, width, precision);
    }
    case 'E': {
        long double d = read_double(args, length);
        flags |= F_UPPERCASE;
        return print_exponential(d, 10, flags, width, precision);
    }
    case 'a': {
        long double d = read_double(args, length);
        return print_exponential(d, 2, flags, width, precision);
    }
    case 'A': {
        long double d = read_double(args, length);
        flags |= F_UPPERCASE;
        return print_exponential(d, 2, flags, width, precision);
    }
    case 'C':
        length |= L_LONG;
        // fall through
    case 'c': {
        char s[MB_CUR_MAX];
        size_t n = read_char(args, length, s);
        if (n == (size_t)-1) {
            return -1;
        }
        return write(STDOUT_FILENO, s, n);
    }
    case 'S':
        length |= L_LONG;
        // fall through
    case 's': {
        char* s = read_string(args, length);
        if (s == NULL) {
            return -1;
        }
        size_t n = strlen(s);
        ssize_t b = write(STDOUT_FILENO, s, n);
        if (length & L_LONG) {
            free(s);
        }
        return b;
    }
    case 'm': {
        char* s = strerror(errno);
        size_t n = strlen(s);
        return write(STDOUT_FILENO, s, n);
    }
    case '%': {
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
