#include <errno.h>
#include <r_args.h>
#include <r_fmtprint.h>
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

int arg_parse(const char* restrict* fmt, Args* args, Flags flags) {
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
    if ((*fmt)[1] == '*') {
        width = *(int*)args_read(args, 0);
        (*fmt)++;
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
        if ((*fmt)[1] == '*') {
            precision = *(int*)args_read(args, 0);
            (*fmt)++;
        }
    }

    // ignore length modifier (read in args_init)
    while ((*fmt)[1] == 'h' || (*fmt)[1] == 'l' || (*fmt)[1] == 'q' ||
           (*fmt)[1] == 'L' || (*fmt)[1] == 'l' || (*fmt)[1] == 'j' ||
           (*fmt)[1] == 'Z' || (*fmt)[1] == 'z' || (*fmt)[1] == 't') {
        (*fmt)++;
    }

    // read conversion
    *fmt += 2;
    switch ((*fmt)[-1]) {
    case 'd':
    case 'i': {
        intmax_t i = *(intmax_t*)args_read(args, 0);
        return print_signed(i, 10, flags, width, precision);
    }
    case 'u': {
        uintmax_t i = *(uintmax_t*)args_read(args, 0);
        return print_unsigned(i, 10, flags, width, precision);
    }
    case 'o': {
        uintmax_t i = *(uintmax_t*)args_read(args, 0);
        return print_unsigned(i, 8, flags, width, precision);
    }
    case 'x': {
        uintmax_t i = *(uintmax_t*)args_read(args, 0);
        return print_unsigned(i, 16, flags, width, precision);
    }
    case 'X': {
        uintmax_t i = *(uintmax_t*)args_read(args, 0);
        flags |= F_UPPERCASE;
        return print_unsigned(i, 16, flags, width, precision);
    }
    case 'p': {
        void* p = *(void**)args_read(args, 0);
        return print_pointer(p, flags, width);
    }
    case 'f': {
        long double d = *(long double*)args_read(args, 0);
        return print_decimal(d, 10, flags, width, precision);
    }
    case 'F': {
        long double d = *(long double*)args_read(args, 0);
        flags |= F_UPPERCASE;
        return print_decimal(d, 10, flags, width, precision);
    }
    case 'e': {
        long double d = *(long double*)args_read(args, 0);
        return print_exponential(d, 10, flags, width, precision);
    }
    case 'E': {
        long double d = *(long double*)args_read(args, 0);
        flags |= F_UPPERCASE;
        return print_exponential(d, 10, flags, width, precision);
    }
    case 'a': {
        long double d = *(long double*)args_read(args, 0);
        return print_exponential(d, 2, flags, width, precision);
    }
    case 'A': {
        long double d = *(long double*)args_read(args, 0);
        flags |= F_UPPERCASE;
        return print_exponential(d, 2, flags, width, precision);
    }
    case 'C':
    case 'c': {
        String* s = (String*)args_read(args, 0);
        ssize_t b = write(STDOUT_FILENO, s->chars, s->size);
        return b;
    }
    case 'S':
    case 's': {
        String* s = (String*)args_read(args, 0);
        ssize_t b = write(STDOUT_FILENO, s->chars, s->size);
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
    va_list vargs;
    va_start(vargs, fmt);
    Args* args = args_init(fmt, &vargs);
    if (args == NULL) {
        return -1;
    }
    int i = 0, b = 0;
    while (fmt[i] != '\0') {
        if (fmt[i] == '%') {
            // arg parse
            if ((b = add_or_error(write(STDOUT_FILENO, fmt, i), b)) < 0) {
                args_end(args);
                return -1;
            }
            fmt += i;
            if ((b = add_or_error(arg_parse(&fmt, args, 0), b)) < 0) {
                args_end(args);
                return -1;
            }
            i = 0;
        } else {
            i++;
        }
    }
    if ((b = add_or_error(write(STDOUT_FILENO, fmt, i), b)) < 0) {
        args_end(args);
        return -1;
    }
    args_end(args);
    return b;
}
