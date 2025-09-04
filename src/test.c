#include <rstd.h>

#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

int main() {
    char* fmt =
        "Hi %c, %lc, %-010.5lld, %0+10ld, % hd, %+020u, %+020.15hhu, %#010lo, "
        "%#010llX, %10p, "
        "%10p, 50%%, "
        "%+20.10Lf, %08.2f, %#.0f, %+5.0f,"
        "%+20.4E, %+#010.0E, %+010.1a, %F, %f, "
        "%s, %ls, %m\n";
    setlocale(LC_ALL, "C.UTF-8");
    int r1 = r_printf(fmt, 'a', L'あ', -222, 222, 222, -1, -1, 15, 15, fmt,
                      NULL, 22.2l, 22.2f, 22.2f, 22.2f, 0.02f, 0.02f, 0.02f,
                      -INFINITY, -NAN, "hello", L"こんにちは日本!");
    int r2 = printf(fmt, 'a', L'あ', -222, 222, 222, -1, -1, 15, 15, fmt, NULL,
                    22.2l, 22.2f, 22.2f, 22.2f, 0.02f, 0.02f, 0.02f, -INFINITY,
                    -NAN, "hello", L"こんにちは日本!");
    printf("r1=%d, r2=%d\n", r1, r2);
    r1 = r_printf("%*.*f\n", 5, 1, 0.2f);
    r2 = printf("%*.*f\n", 5, 1, 0.2f);
    printf("r1=%d, r2=%d\n", r1, r2);
    r1 = r_printf("%2$*4$.*5$f, %3$s, %1$d\n", 2, 0.2f, "hello", 5, 1);
    r2 = printf("%2$*4$.*5$f, %3$s, %1$d\n", 2, 0.2f, "hello", 5, 1);
    printf("r1=%d, r2=%d\n", r1, r2);
    r_printf("%s, %werror", "This used to leak memory");
    printf("r_abs(-36)=%d\n", r_abs(-36));
    printf("r_gcd(4, -36)=%d\n", r_gcd(4, -36));
    printf("r_lcm(4, -36)=%d\n", r_lcm(4, -36));

    char* buf = r_alloc_n(r_libc_allocator, char, 10);
    char str[10] = "test str\n";
    memcpy(buf, str, 10 * sizeof(char));
    printf("%s", buf);
    r_free_n(r_libc_allocator, buf, 10);
}
