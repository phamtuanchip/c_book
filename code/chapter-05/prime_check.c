#include <stdio.h>
#include <math.h>
#include <stdbool.h>

bool is_prime(long n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0) return false;
    long r = (long)sqrt((double)n);
    for (long i = 3; i <= r; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

int main(void) {
    long n;
    printf("Nhap so nguyen duong: ");
    if (scanf("%ld", &n) != 1) { fprintf(stderr, "Nhap khong hop le.\n"); return 1; }
    if (is_prime(n)) printf("%ld la so nguyen to\n", n);
    else printf("%ld khong la so nguyen to\n", n);
    return 0;
}
