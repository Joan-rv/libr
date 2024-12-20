#include <r_args.h>
#include <r_printf.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wchar.h>

struct Args {
    void** args;
    size_t size;
    int arg_pos;
};

Length read_length_modifier(const char* restrict* fmt) {
    Length length = 0;
    while (true) {
        switch (**fmt) {
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

intmax_t read_signed(va_list* vargs, Length length) {
    if (length & L_CHAR) {
        return (char)va_arg(*vargs, int);
    } else if (length & L_SHORT) {
        return (short)va_arg(*vargs, int);
    } else if (length & L_LONG) {
        return (long)va_arg(*vargs, long);
    } else if (length & L_LONGLONG) {
        return (long long)va_arg(*vargs, long long);
    } else if (length & L_INTMAX) {
        return va_arg(*vargs, intmax_t);
    } else if (length & L_SIZET) {
        return (ssize_t)va_arg(*vargs, ssize_t);
    } else if (length & L_PTRDIFF) {
        return (ptrdiff_t)va_arg(*vargs, ptrdiff_t);
    } else {
        return (int)va_arg(*vargs, int);
    }
}

uintmax_t read_unsigned(va_list* vargs, Length length) {
    if (length & L_CHAR) {
        return (unsigned char)va_arg(*vargs, unsigned int);
    } else if (length & L_SHORT) {
        return (unsigned short)va_arg(*vargs, unsigned int);
    } else if (length & L_LONGLONG) {
        return (unsigned long long)va_arg(*vargs, unsigned long long);
    } else if (length & L_INTMAX) {
        return va_arg(*vargs, uintmax_t);
    } else if (length & L_SIZET) {
        return (size_t)va_arg(*vargs, size_t);
    } else if (length & L_PTRDIFF) {
        return (ptrdiff_t)va_arg(*vargs, ptrdiff_t);
    } else {
        return (unsigned int)va_arg(*vargs, int);
    }
}

long double read_double(va_list* vargs, Length length) {
    if (length & L_LONGLONG) {
        return va_arg(*vargs, long double);
    } else {
        return (double)va_arg(*vargs, double);
    }
}

String* read_char(va_list* vargs, Length length) {
    String* s = malloc(sizeof(String));
    if (s == NULL) {
        return NULL;
    }
    if (length & L_LONG) {
        s->chars = malloc(MB_CUR_MAX * sizeof(char));
        if (s->chars == NULL) {
            return NULL;
        }
        wint_t c = va_arg(*vargs, wint_t);
        mbstate_t state;
        memset(&state, 0, sizeof(state));
        s->size = wcrtomb(s->chars, c, &state);
        if (s->size == (size_t)-1) {
            return NULL;
        }
    } else {
        s->chars = malloc(sizeof(char));
        if (s->chars == NULL) {
            return NULL;
        }
        s->chars[0] = (char)va_arg(*vargs, int);
        s->size = 1;
    }
    return s;
}

String* read_string(va_list* vargs, Length length) {
    String* s = malloc(sizeof(String));
    if (s == NULL) {
        return NULL;
    }
    if (length & L_LONG) {
        wchar_t* ws = va_arg(*vargs, wchar_t*);
        s->size = wcstombs(NULL, ws, 0);
        if (s->size == (size_t)-1) {
            return NULL;
        }
        s->chars = malloc(s->size + 1);
        if (s->chars == NULL) {
            return NULL;
        }
        wcstombs(s->chars, ws, s->size + 1);
    } else {
        char* s_arg = va_arg(*vargs, char*);
        if (s == NULL) {
            return NULL;
        }
        s->size = strlen(s_arg);
        s->chars = malloc((s->size + 1) * sizeof(char));
        if (s->chars == NULL) {
            return NULL;
        }
        strcpy(s->chars, s_arg);
    }
    return s;
}

void* read_arg(const char* restrict* fmt, va_list* vargs) {
    (*fmt)++;
    while (('0' <= **fmt && **fmt <= '9') || **fmt == '.' || **fmt == '*' ||
           **fmt == '$' || **fmt == '#' || **fmt == '-' || **fmt == ' ' ||
           **fmt == '+') {
        (*fmt)++;
    }
    Length length = read_length_modifier(fmt);
    switch (**fmt) {
    case 'd':
    case 'i': {
        intmax_t* arg = malloc(sizeof(intmax_t));
        if (arg != NULL) {
            *arg = read_signed(vargs, length);
        }
        return arg;
    }
    case 'u':
    case 'o':
    case 'x':
    case 'X': {
        uintmax_t* arg = malloc(sizeof(uintmax_t));
        if (arg != NULL) {

            *arg = read_signed(vargs, length);
        }
        return arg;
    }
    case 'p': {
        void** arg = malloc(sizeof(void*));
        if (arg != NULL) {
            *arg = va_arg(*vargs, void*);
        }
        return arg;
    }
    case 'f':
    case 'F':
    case 'e':
    case 'E':
    case 'a':
    case 'A': {
        long double* arg = malloc(sizeof(long double));
        if (arg != NULL) {
            *arg = read_double(vargs, length);
        }
        return arg;
    }
    case 'C':
        length |= L_LONG;
        // fall through
    case 'c': {
        return read_char(vargs, length);
    }
    case 'S':
        length |= L_LONG;
        // fall through
    case 's': {
        return read_string(vargs, length);
    }
    default:
        return NULL;
    }
}

Args* args_init(const char* restrict fmt, va_list* vargs) {
    Args* args = malloc(sizeof(Args));
    if (args == NULL) {
        return NULL;
    }
    args->args = NULL;
    args->size = 0;
    args->arg_pos = 0;
    while (*fmt != '\0') {
        if (*fmt == '%') {
            if (fmt[1] != '%' && fmt[1] != 'm') {
                if (!(args->args = realloc(args->args,
                                           (args->size + 1) * sizeof(void*)))) {
                    args_end(args);
                    return NULL;
                }
                if (!(args->args[args->size] = read_arg(&fmt, vargs))) {
                    args_end(args);
                    return NULL;
                }
                args->size += 1;
            } else {
                fmt++;
            }
        }
        fmt++;
    }
    va_end(*vargs);
    return args;
}

void* args_read(Args* args, int pos) {
    if (pos == 0) {
        args->arg_pos++;
    }
    return args->args[args->arg_pos - 1];
}

void args_end(Args* args) {
    // TODO: memory allocated for strings is leaked, we should free it here
    for (size_t i = 0; i < args->size; i++) {
        free(args->args[i]);
    }
    free(args->args);
    free(args);
}
