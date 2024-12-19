#include <sys/types.h>

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
    L_INTMAX = 1 << 5,
    L_SIZET = 1 << 6,
    L_PTRDIFF = 1 << 7,
} Length;

int add_or_error(ssize_t r, int b);
