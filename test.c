#include <libr.h>
#include <stdio.h>

int main() {
    char* fmt = "Hi %c, %d, 50%%\n";
    int r1 = r_printf(fmt, 'a', -222);
    int r2 = printf(fmt, 'a', -222);
    printf("r1=%d, r2=%d\n", r1, r2);
}
