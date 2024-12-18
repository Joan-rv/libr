#include <libr.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>

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
    printf("r_abs(-36)=%d\n", r_abs(-36));
    printf("r_gcd(4, -36)=%d\n", r_gcd(4, -36));
    printf("r_lcm(4, -36)=%d\n", r_lcm(4, -36));
}
