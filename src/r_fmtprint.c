#include "r_fmtprint.h"
#include "r_printf.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

int print_decimal(long double n, int base, Flags flags, int width,
                  int precision) {
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

int print_exponential(long double n, int base, Flags flags, int width,
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
