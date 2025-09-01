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

/* -------- ALLOCATION -------- */
typedef struct r_allocator {
    void* (*alloc)(size_t size, size_t alignment, void* ctx);
    void (*free)(void* ptr, size_t size, void* ctx);
    void* ctx;
} r_allocator_t;

#define r_new(a, T) a.alloc(sizeof(T), __alignof(T), a.ctx)
#define r_new_n(a, T, n) a.alloc(n * sizeof(T), __alignof(T), a.ctx)
#define r_delte(a, p) a.free((p), sizeof(__typeof(*(p))), a.ctx)
#define r_delte_n(a, p, n) a.free((p), n * sizeof(__typeof(*(p))), a.ctx)

extern const r_allocator_t r_libc_allocator;

/* -------- UTILS -------- */
#define R_UNUSED(v) (void)(v)

#endif
