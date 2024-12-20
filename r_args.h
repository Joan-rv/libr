#ifndef R_ARGS_H
#define R_ARGS_H
#include <stdarg.h>
#include <sys/types.h>

typedef struct Args Args;
typedef struct {
    char* chars;
    size_t size;
} String;

Args* args_init(const char* restrict fmt, va_list* vargs);
void args_end(Args* args);
void* args_read(Args* args, int pos);
#endif
