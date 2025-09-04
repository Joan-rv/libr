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
typedef struct r_allocator_vtable {
    void* (*alloc)(size_t size, size_t alignment, void* ctx);
    void (*free)(void* ptr, size_t size, void* ctx);
} r_allocator_vtable_t;

typedef struct r_allocator {
    const r_allocator_vtable_t* vtable;
    void* ctx;
} r_allocator_t;

#define r_alloc(a, T) (T*)a.vtable->alloc(sizeof(T), __alignof(T), a.ctx)
#define r_alloc_n(a, T, n)                                                     \
    (T*)a.vtable->alloc(n * sizeof(T), __alignof(T), a.ctx)
#define r_free(a, p) a.vtable->free((p), sizeof(*(p)), a.ctx)
#define r_free_n(a, p, n) a.vtable->free((p), n * sizeof(*(p)), a.ctx)

extern const r_allocator_t r_libc_allocator;

/* -------- UTILS -------- */
#define R_UNUSED(v) (void)(v)

#endif
