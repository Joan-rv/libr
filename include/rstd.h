#ifndef RSTD_H
#define RSTD_H

#include <stdarg.h>
#include <stddef.h>

/* -------- IO -------- */
int r_printf(const char* restrict fmt, ...);
int r_vprintf(const char* restrict fmt, va_list ap);

/* -------- MATH --------*/
int r_abs(int n);
int r_gcd(int a, int b);
int r_lcm(int a, int b);

#endif
