#include <libr.h>
#include <math.h>
#include <stdio.h>

int main() {
    char* fmt = "Hi %c, %d, %u, %o, %x, 50%%, %f, %f, %f, %s\n";
    int r1 =
        r_printf(fmt, 'a', -222, -1, 15, 15, -2.2f, -INFINITY, -NAN, "hello");
    int r2 =
        printf(fmt, 'a', -222, -1, 15, 15, -2.2f, -INFINITY, -NAN, "hello");
    printf("r1=%d, r2=%d\n", r1, r2);
    printf("r_gcd(4, -36)=%d\n", r_gcd(4, -18));
}
