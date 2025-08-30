#ifndef RSTD_H
#define RSTD_H

/* -------- IO -------- */
int r_printf(const char* restrict fmt, ...);

/* -------- MATH --------*/
int r_abs(int n);
int r_gcd(int a, int b);
int r_lcm(int a, int b);

#endif
