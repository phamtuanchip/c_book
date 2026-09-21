# Chương 5 — Lời giải bài tập

## Bài 1: bảng cửu chương

```c
// times_table.c
#include <stdio.h>

int main(void) {
    for (int i = 1; i <= 9; i++) {
        for (int j = 1; j <= 10; j++) printf("%d x %2d = %3d\n", i, j, i * j);
        printf("\n");
    }
    return 0;
}
```

## Bài 2: số nguyên tố ≤ 100 và số lớn nhất < 1.000.000

Hàm `is_prime` đã có trong mục 5.10 (`prime_check.c`). Để tìm số nguyên tố lớn nhất nhỏ hơn 1.000.000, đi **ngược** từ 999.999 và dừng ở số đầu tiên thỏa:

```c
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
```

Kết quả: `999983`.

## Bài 3: menu chịu được đầu vào chữ

Xem `menu.c` ở mục 5.10 — nó dọn phần còn lại của dòng khi `scanf` thất bại và xử lý EOF để không lặp vô hạn.

## Bài 4: đoán số

```c
// guess.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand((unsigned)time(NULL));
    int secret = rand() % 100 + 1;                       // 1..100 (hơi lệch nhẹ nhưng đủ cho trò chơi)
    int guess, tries = 0;

    printf("Toi nghi ra mot so tu 1 den 100. Doan di!\n");
    do {
        printf("Doan: ");
        if (scanf("%d", &guess) != 1) {                  // chữ hoặc EOF
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
            if (c == EOF) return 1;
            printf("Hay nhap mot so.\n");
            guess = 0;
            continue;                                    // trong do-while: nhảy tới kiểm tra điều kiện
        }
        tries++;
        if (guess < secret)      printf("Lon hon!\n");
        else if (guess > secret) printf("Nho hon!\n");
    } while (guess != secret);

    printf("Dung roi sau %d lan doan.\n", tries);
    return 0;
}
```

Chú ý: khi nhập sai, `guess = 0` và `continue` quay về điều kiện `guess != secret` (luôn đúng vì `secret ≥ 1`) nên vòng lặp tiếp tục.

## Bài 5: Fibonacci và giai thừa

```c
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
```

Với `int` 32 bit, `13!` đã tràn (`12! = 479001600` là lớn nhất). Cách kiểm tra tràn **trước** khi nhân (`f > MAX / n`) tránh UB/quay vòng.

## Bài 6: min, max, trung bình đến hết đầu vào

```c
// stats_stdin.c
#include <stdio.h>

int main(void) {
    int x, count = 0, min = 0, max = 0;
    long sum = 0;
    while (scanf("%d", &x) == 1) {
        if (count == 0 || x < min) min = x;
        if (count == 0 || x > max) max = x;
        sum += x;
        count++;
    }
    if (count == 0) { printf("khong co so nao\n"); return 0; }
    printf("min = %d, max = %d, trung binh = %.2f\n", min, max, (double)sum / count);
    return 0;
}
```

## Bài 7: tam giác Pascal

```c
// pascal.c
#include <stdio.h>

int main(void) {
    int rows = 8;
    for (int n = 0; n < rows; n++) {
        for (int s = 0; s < (rows - n - 1) * 2; s++) putchar(' ');   // căn giữa
        long c = 1;                                                   // C(n, 0)
        for (int k = 0; k <= n; k++) {
            printf("%4ld", c);
            c = c * (n - k) / (k + 1);                                // C(n, k+1) = C(n, k) * (n - k) / (k + 1)
        }
        printf("\n");
    }
    return 0;
}
```

Phép nhân trước rồi mới chia luôn cho kết quả nguyên vì `C(n,k) × (n−k)` chia hết cho `k+1`.

## Bài 8: tìm lỗi vòng lặp vô hạn

```c
int i = 0;
while (i <= 10) {
    if (i % 2 != 0) continue;   // BUG
    printf("%d\n", i);
    i++;
}
```

Khi `i` = 1 (lẻ) thì `continue` bỏ qua `i++`, nên `i` mãi bằng 1. (Thực ra `i` bắt đầu từ 0 chẵn nên in `0`, `i++` → 1, rồi kẹt ở 1.) Sửa bằng cách chuyển bước cập nhật vào `for`:

```c
for (int i = 0; i <= 10; i += 2) printf("%d\n", i);
```

hoặc đảo điều kiện: `if (i % 2 == 0) printf("%d\n", i); i++;` (bước cập nhật luôn chạy).

## Bài 9: thập phân sang nhị phân

```c
// dec2bin.c
#include <stdio.h>

int main(void) {
    unsigned n;
    if (scanf("%u", &n) != 1) return 1;
    if (n == 0) { puts("0"); return 0; }

    char bits[33];
    int len = 0;
    while (n > 0) {
        bits[len++] = (char)('0' + (n % 2));      // các bit ra theo thứ tự ngược
        n /= 2;
    }
    for (int i = len - 1; i >= 0; i--) putchar(bits[i]);
    putchar('\n');
    return 0;
}
```
