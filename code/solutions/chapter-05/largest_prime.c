// largest_prime.c
#include <stdbool.h>
#include <stdio.h>

static bool is_prime(long n) {
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0) return false;
    for (long i = 3; i * i <= n; i += 2)
        if (n % i == 0) return false;
    return true;
}

int main(void) {
    for (long n = 999999; n >= 2; n--) {
        if (is_prime(n)) { printf("%ld\n", n); break; }    // 999983
    }
    return 0;
}
