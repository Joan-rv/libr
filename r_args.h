#include <stdarg.h>

typedef struct Args Args;
Args* args_init(const char* restrict fmt, va_list* vargs);
void args_end(Args* args);
void* args_read(Args* args, int pos);
