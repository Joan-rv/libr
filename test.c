#include <libr.h>
#include <math.h>
#include <stdio.h>

int main() {
    char* fmt =
        "Hi %c, %+d, % d, %+u, %o, %X, %p, %p, 50%%, %+f, %+E, %F, %f, %s,\n";
    int r1 = r_printf(fmt, 'a', 222, 222, -1, 15, 15, fmt, NULL, 22.2f, 0.02f,
                      -INFINITY, -NAN, "hello");
    int r2 = printf(fmt, 'a', 222, 222, -1, 15, 15, fmt, NULL, 22.2f, 0.02f,
                    -INFINITY, -NAN, "hello");
    printf("r1=%d, r2=%d\n", r1, r2);
    printf("r_gcd(4, -36)=%d\n", r_gcd(4, -18));
}
