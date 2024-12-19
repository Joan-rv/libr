#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
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

typedef struct Args Args;
Args* args_init(const char* restrict fmt, va_list* vargs);
void args_end(Args* args);
void* args_read(Args* args, int pos);

int add_or_error(ssize_t r, int b);
int print_unsigned(uintmax_t n, unsigned int base, Flags flags, int width,
                   int precision);
int print_signed(intmax_t n, int base, Flags flags, int width, int precision);
int print_pointer(void* p, Flags flags, int width);
bool handle_nan_or_inf(double n, Flags flags, int* b);
int print_decimal(long double n, int base, Flags flags, int width,
                  int precision);
int print_exponential(long double n, int base, Flags flags, int width,
                      int precision);
