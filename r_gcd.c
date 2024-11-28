#include <stdlib.h>
// Use Euclid's algorithm to find the gcd
int r_gcd(int a, int b) {
    if (a == 0) {
        return abs(b);
    } else {
        return r_gcd(b % a, a);
    }
}
