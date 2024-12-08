#include <libr.h>
#include <math.h>
#include <stdio.h>

int main() {
    char* fmt =
        "Hi %c, %-10.5d, %+10d, % d, %+20.15u, %#10o, %#X, %10p, %10p, 50%%, "
        "%+20.10f, %#.0f, %+5.0f,"
        "%+20.4E, %F, %f, "
        "%s, %m\n";
    int r1 = r_printf(fmt, 'a', -222, 222, 222, -1, 15, 15, fmt, NULL, 22.2f,
                      22.2f, 22.2f, 0.02f, -INFINITY, -NAN, "hello");
    int r2 = printf(fmt, 'a', -222, 222, 222, -1, 15, 15, fmt, NULL, 22.2f,
                    22.2f, 22.2f, 0.02f, -INFINITY, -NAN, "hello");
    printf("r1=%d, r2=%d\n", r1, r2);
    printf("r_abs(-36)=%d\n", r_abs(-36));
    printf("r_gcd(4, -36)=%d\n", r_gcd(4, -36));
    printf("r_lcm(4, -36)=%d\n", r_lcm(4, -36));
}
