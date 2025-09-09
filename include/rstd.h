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

#define r_alloc(T, a) (T*)(a).vtable->alloc(sizeof(T), __alignof(T), (a).ctx)
#define r_alloc_n(T, n, a)                                                     \
    (T*)(a).vtable->alloc(n * sizeof(T), __alignof(T), (a).ctx)
#define r_realloc(ptr, old_n, new_n, a)                                        \
    (__typeof(ptr))(a).vtable->remap(ptr, __alignof(*ptr), old_n, new_n,       \
                                     (a).ctx)
#define r_free(p, a) (a).vtable->free((p), sizeof(*(p)), (a).ctx)
#define r_free_n(p, n, a) (a).vtable->free((p), n * sizeof(*(p)), (a).ctx)

extern const r_allocator_t r_libc_allocator;

/* ------- DYNAMIC ARRAYS -------- */
#define R_DA_INIT_CAP 8

#define R_ARR_DECLARE(type, name)                                              \
    typedef struct name##_da {                                                 \
        type* data;                                                            \
        size_t len, cap;                                                       \
    } name##_da_t;                                                             \
    typedef struct name##_va {                                                 \
        type* data;                                                            \
        size_t len;                                                            \
    } name##_va_t;                                                             \
    bool name##_reserve(name##_da_t* da, size_t cap, r_allocator_t a);         \
    bool name##_append(name##_da_t* da, type v, r_allocator_t a);              \
    void name##_free(name##_da_t* da, r_allocator_t a);                        \
    name##_va_t name##_as_view(const name##_da_t da);

#define R_ARR_DEFINE(type, name)                                               \
    bool name##_reserve(name##_da_t* da, size_t cap, r_allocator_t a) {        \
        if (da->cap >= cap)                                                    \
            return true;                                                       \
        size_t new_cap = da->cap == 0 ? R_DA_INIT_CAP : da->cap * 2;           \
        if (new_cap < cap)                                                     \
            new_cap = cap;                                                     \
        type* new_data = r_realloc(da->data, da->cap, new_cap, a);             \
        if (!new_data)                                                         \
            return false;                                                      \
        da->data = new_data;                                                   \
        da->cap = new_cap;                                                     \
        return true;                                                           \
    }                                                                          \
    bool name##_append(name##_da_t* da, type v, r_allocator_t a) {             \
        if (!name##_reserve(da, da->len + 1, a))                               \
            return false;                                                      \
        da->data[da->len++] = v;                                               \
        return true;                                                           \
    }                                                                          \
    void name##_free(name##_da_t* da, r_allocator_t a) {                       \
        r_free_n(da->data, da->cap, a);                                        \
        *da = (name##_da_t){0};                                                \
    }                                                                          \
    name##_va_t name##_as_view(const name##_da_t da) {                         \
        return (name##_va_t){.data = da.data, .len = da.len};                  \
    }

/* -------- UTILS -------- */
#define R_UNUSED(v) (void)(v)

#endif
