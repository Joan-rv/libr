#include <rstd.h>

int r_abs(int n) {
    if (n > 0) {
        return n;
    } else {
        return -n;
    }
}

// Use Euclid's algorithm to find the gcd
int r_gcd(int a, int b) {
    if (a == 0) {
        return r_abs(b);
    } else {
        return r_gcd(b % a, a);
    }
}

int r_lcm(int a, int b) { return r_abs(a * b) / r_gcd(a, b); }
