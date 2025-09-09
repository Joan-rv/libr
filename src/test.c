#include <rstd.h>

#include <assert.h>
#include <errno.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

R_ARR_DECLARE(int, arr)
R_ARR_DEFINE(int, arr)

int main(void) {
    char* ptr = (char*)0xcafebabe;
    char* fmt =
        "Hi %c, %lc, %-010.5lld, %0+10ld, % hd, %+020u, %+020.15hhu, %#010lo, "
        "%#010llX, %10p, "
        "%10p, 50%%, "
        "%+20.10Lf, %08.2f, %#.0f, %+5.0f,"
        "%+20.4E, %+#010.0E, %+010.1a, %F, %f, "
        "%s, %ls, %m\n";
    setlocale(LC_ALL, "C.UTF-8");
    errno = 0;
    int r1 = r_printf(fmt, 'a', L'あ', -222, 222, 222, -1, -1, 15, 15, ptr,
                      NULL, 22.2l, 22.2f, 22.2f, 22.2f, 0.02f, 0.02f, 0.02f,
                      -INFINITY, -NAN, "hello", L"こんにちは日本!");
    int r2 = printf(fmt, 'a', L'あ', -222, 222, 222, -1, -1, 15, 15, ptr, NULL,
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

    const r_allocator_t a = r_libc_allocator;
    char* buf = r_alloc_n(char, 10, a);
    char str[10] = "test str\n";
    memcpy(buf, str, 10 * sizeof(char));
    buf = r_realloc(buf, 10, 12, a);
    buf[9] = '1';
    buf[10] = '\n';
    buf[11] = '\0';
    printf("%s", buf);
    r_free_n(buf, 10, a);

    arr_da_t test_da = {0};
    for (int i = -10; i <= 10; ++i) {
        arr_append(&test_da, i, a);
    }
    if (test_da.len > 0)
        printf("%d", test_da.data[0]);
    for (size_t i = 1; i < test_da.len; ++i) {
        printf(", %d", test_da.data[i]);
    }
    putchar('\n');
    arr_free(&test_da, a);

    r_str_da_t s = {0};
    for (char c = 'a'; c <= 'g'; c++) {
        r_str_append(&s, c, a);
    }
    assert(r_str_eq(r_str_as_view(s), R_STR("abcdefg")));
    r_str_free(&s, a);
}
