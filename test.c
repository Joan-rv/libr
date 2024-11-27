#include <libr.h>
#include <stdio.h>

int main() {
    char* fmt = "Hi %c, %d, 50%%\n";
    r_printf(fmt, 'a', -222);
    printf(fmt, 'a', -222);
}
