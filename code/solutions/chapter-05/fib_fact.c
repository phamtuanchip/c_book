// fib_fact.c
#include <stdio.h>

int main(void) {
    // Fibonacci < 1000
    long a = 0, b = 1;
    while (a < 1000) {
        printf("%ld ", a);
        long t = a + b;
        a = b;
        b = t;
    }
    printf("\n");

    // n! với unsigned long long: 20! = 2432902008176640000 vừa 64 bit, 21! tràn
    unsigned long long f = 1;
    for (int n = 1; n <= 25; n++) {
        if (f > 18446744073709551615ULL / (unsigned long long)n) {
            printf("%d! tran unsigned long long\n", n);      // n = 21
            break;
        }
        f *= (unsigned long long)n;
        printf("%2d! = %llu\n", n, f);
    }
    return 0;
}
