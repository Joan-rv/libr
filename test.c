#include <libr.h>
#include <math.h>
#include <stdio.h>

int main() {
    char* fmt = "Hi %c, %-010.5d, %0+10d, % d, %+020u, %+020.15u, %#010o, "
                "%#010X, %10p, "
                "%10p, 50%%, "
                "%+20.10f, %08.2f, %#.0f, %+5.0f,"
                "%+20.4E, %+010.0E, %F, %f, "
                "%s, %m\n";
    int r1 =
        r_printf(fmt, 'a', -222, 222, 222, -1, -1, 15, 15, fmt, NULL, 22.2f,
                 22.2f, 22.2f, 22.2f, 0.02f, 0.02f, -INFINITY, -NAN, "hello");
    int r2 =
        printf(fmt, 'a', -222, 222, 222, -1, -1, 15, 15, fmt, NULL, 22.2f,
               22.2f, 22.2f, 22.2f, 0.02f, 0.02f, -INFINITY, -NAN, "hello");
    printf("r1=%d, r2=%d\n", r1, r2);
    printf("r_abs(-36)=%d\n", r_abs(-36));
    printf("r_gcd(4, -36)=%d\n", r_gcd(4, -36));
    printf("r_lcm(4, -36)=%d\n", r_lcm(4, -36));
}
