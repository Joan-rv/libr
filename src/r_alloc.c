#include <rstd.h>

#include <stdlib.h>

static void* libc_alloc(size_t size, size_t alignment, void* ctx) {
    R_UNUSED(alignment);
    R_UNUSED(ctx);
    return malloc(size);
}

static bool libc_resize(void* ptr, size_t alignment, size_t old_size,
                        size_t new_size, void* ctx) {
    R_UNUSED(alignment);
    R_UNUSED(ptr);
    R_UNUSED(ctx);
    return new_size <= old_size;
}

static void* libc_remap(void* ptr, size_t alignment, size_t old_size,
                        size_t new_size, void* ctx) {
    R_UNUSED(alignment);
    R_UNUSED(old_size);
    R_UNUSED(ctx);
    if (new_size == 0) {
        free(ptr);
        return NULL;
    } else {
        return realloc(ptr, new_size);
    }
}

static void libc_free(void* ptr, size_t size, void* ctx) {
    R_UNUSED(ctx);
    R_UNUSED(size);
    free(ptr);
}

const r_allocator_vtable_t r_libc_allocator_vtable = {
    .alloc = libc_alloc,
    .resize = libc_resize,
    .remap = libc_remap,
    .free = libc_free,
};

const r_allocator_t r_libc_allocator = {
    .vtable = &r_libc_allocator_vtable,
    .ctx = NULL,
};
