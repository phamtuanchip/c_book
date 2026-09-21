/* Simple CPU-bound loop to profile
Build: gcc -O2 -g -Wall -o profile_demo profile_demo.c
Use: gprof (with -pg) or perf/valgrind callgrind to profile.
*/
#include <stdio.h>
#include <stdlib.h>

long fib(int n) { if (n < 2) return n; return fib(n-1)+fib(n-2); }

int main(void) {
    for (int i = 30; i < 35; ++i) {
        long v = fib(i);
        printf("fib(%d)=%ld\n", i, v);
    }
    return 0;
}
