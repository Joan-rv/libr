#ifndef RSTD_H
#define RSTD_H

#include <stdarg.h>
#include <stdbool.h>
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
    bool (*resize)(void* ptr, size_t alignment, size_t old_size,
                   size_t new_size, void* ctx);
    void* (*remap)(void* ptr, size_t alignment, size_t old_size,
                   size_t new_size, void* ctx);
    void (*free)(void* ptr, size_t size, void* ctx);
} r_allocator_vtable_t;

typedef struct r_allocator {
    const r_allocator_vtable_t* vtable;
    void* ctx;
} r_allocator_t;

#define r_alloc(a, T) (T*)(a).vtable->alloc(sizeof(T), __alignof(T), (a).ctx)
#define r_alloc_n(a, T, n)                                                     \
    (T*)(a).vtable->alloc(n * sizeof(T), __alignof(T), (a).ctx)
#define r_realloc(a, ptr, old_n, new_n)                                        \
    (__typeof(ptr))(a).vtable->remap(ptr, __alignof(*ptr), old_n, new_n,       \
                                     (a).ctx)
#define r_free(a, p) (a).vtable->free((p), sizeof(*(p)), (a).ctx)
#define r_free_n(a, p, n) (a).vtable->free((p), n * sizeof(*(p)), (a).ctx)

extern const r_allocator_t r_libc_allocator;

/* ------- DYNAMIC ARRAYS -------- */
#define R_DA_INIT_CAP 8
#define r_da_reserve(a, da, n)                                                 \
    do {                                                                       \
        if (n > (da)->cap) {                                                   \
            size_t new_cap = (da)->cap ? 2 * (da)->cap : R_DA_INIT_CAP;        \
            if (new_cap < n)                                                   \
                new_cap = n;                                                   \
            void* new_data = r_alloc_n(a, __typeof(*(da)->data), new_cap);     \
            if (new_data) {                                                    \
                if ((da)->data)                                                \
                    memcpy(new_data, (da)->data,                               \
                           (da)->len * sizeof(*(da)->data));                   \
                r_free_n(a, (da)->data, (da)->cap);                            \
                (da)->data = new_data;                                         \
                (da)->cap = new_cap;                                           \
            }                                                                  \
        }                                                                      \
    } while (0)

#define r_da_append(a, da, v)                                                  \
    do {                                                                       \
        r_da_reserve(a, da, (da)->len + 1);                                    \
        if ((da)->cap > (da)->len)                                             \
            (da)->data[(da)->len++] = v;                                       \
    } while (0)

#define r_da_free(a, da)                                                       \
    do {                                                                       \
        r_free_n(a, (da)->data, (da)->cap);                                    \
        (da)->data = NULL;                                                     \
        (da)->len = 0;                                                         \
        (da)->cap = 0;                                                         \
    } while (0)

/* -------- UTILS -------- */
#define R_UNUSED(v) (void)(v)

#endif
