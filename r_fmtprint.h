#ifndef R_FMTPRINT_H
#define R_FMTPRINT_H
#include <r_printf.h>
#include <stdbool.h>
#include <stdint.h>

int print_unsigned(uintmax_t n, unsigned int base, Flags flags, int width,
                   int precision);
int print_signed(intmax_t n, int base, Flags flags, int width, int precision);
int print_pointer(void* p, Flags flags, int width);
bool handle_nan_or_inf(double n, Flags flags, int* b);
int print_decimal(long double n, int base, Flags flags, int width,
                  int precision);
int print_exponential(long double n, int base, Flags flags, int width,
                      int precision);
#endif
