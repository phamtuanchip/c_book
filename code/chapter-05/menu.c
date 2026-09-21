// menu.c
#include <stdio.h>
#include <stdbool.h>

static void print_table(int n) {
    for (int i = 1; i <= 10; i++) printf("%d x %2d = %3d\n", n, i, n * i);
}

static bool is_prime(long n) {
    if (n < 2) return false;
    for (long i = 2; i * i <= n; i++) if (n % i == 0) return false;
    return true;
}

int main(void) {
    int choice;
    do {
        printf("\n===== MENU =====\n");
        printf("[1] In bang cuu chuong\n");
        printf("[2] Kiem tra so nguyen to\n");
        printf("[0] Thoat\n");
        printf("Chon: ");

        if (scanf("%d", &choice) != 1) {           // người dùng nhập chữ
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
            if (c == EOF) break;
            printf("Lua chon khong hop le.\n");
            choice = -1;
            continue;                              // trong do-while, continue nhảy tới kiểm tra điều kiện
        }

        switch (choice) {
            case 1: {
                int n;
                printf("Nhap so: ");
                if (scanf("%d", &n) == 1) print_table(n);
                break;
            }
            case 2: {
                long n;
                printf("Nhap so: ");
                if (scanf("%ld", &n) == 1)
                    printf("%ld %s so nguyen to\n", n, is_prime(n) ? "la" : "khong phai la");
                break;
            }
            case 0:
                printf("Tam biet!\n");
                break;
            default:
                printf("Lua chon khong hop le.\n");
        }
    } while (choice != 0);

    return 0;
}
