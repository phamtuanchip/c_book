// prime_check.c
#include <stdio.h>
#include <stdbool.h>

bool is_prime(long n) {
    if (n < 2) return false;
    if (n < 4) return true;               // 2 và 3
    if (n % 2 == 0) return false;         // loại số chẵn
    for (long i = 3; i * i <= n; i += 2) { // chỉ thử ước lẻ; i*i <= n thay cho sqrt, tránh dùng số thực
        if (n % i == 0) return false;     // thoát sớm khi tìm thấy ước
    }
    return true;
}

int main(void) {
    printf("Cac so nguyen to tu 2 den 100:\n");
    int count = 0;
    for (int n = 2; n <= 100; n++) {
        if (is_prime(n)) {
            printf("%d ", n);
            count++;
        }
    }
    printf("\n(%d so)\n", count);        // 25 số
    return 0;
}
