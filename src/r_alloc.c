#include <rstd.h>

#include <stdlib.h>

static void* libc_alloc(size_t size, size_t alignment, void* ctx) {
    R_UNUSED(alignment);
    R_UNUSED(ctx);
    return malloc(size);
}

static void libc_free(void* ptr, size_t size, void* ctx) {
    R_UNUSED(ctx);
    R_UNUSED(size);
    free(ptr);
}

const r_allocator_t r_libc_allocator = {
    .alloc = libc_alloc,
    .free = libc_free,
    .ctx = NULL,
};
