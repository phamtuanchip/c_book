# Chương 5 — Điều khiển luồng (Control Flow)

## Mục tiêu chương

- Hiểu **luồng thực thi** của chương trình và vì sao cần cấu trúc điều khiển.
- Dùng thành thạo `if / else if / else`, `switch`, `for`, `while`, `do-while`.
- Dùng `break`, `continue` và (hạn chế) `goto` đúng chỗ.
- Nhận diện và tránh lỗi kinh điển: vòng lặp vô hạn, lỗi lệch một (*off-by-one*), `switch` thiếu `break`, `unsigned` trong điều kiện lặp.
- Áp dụng vào bài toán thực tế: kiểm tra số nguyên tố, bảng cửu chương, menu tương tác, đọc đến hết đầu vào.

## 5.1. Luồng thực thi là gì?

Mặc định, chương trình chạy **tuần tự** từ trên xuống dưới. Cấu trúc điều khiển cho phép:

- **Rẽ nhánh (selection):** làm việc này *hoặc* việc kia tùy điều kiện — `if`, `switch`.
- **Lặp (iteration):** làm đi làm lại một việc — `for`, `while`, `do-while`.
- **Nhảy (jump):** chuyển luồng đến nơi khác — `break`, `continue`, `return`, `goto`.

```text
          ┌─────────┐
          │ điều kiện? │──đúng──► [làm A] ──┐
          └────┬────┘                     │
               │sai                        ▼
               └──────────► [làm B] ──► tiếp tục
```

### "Đúng" và "sai" trong C

C (trước C23) không có kiểu boolean nguyên thủy trong điều kiện: **giá trị `0` là sai, mọi giá trị khác 0 là đúng.** Các toán tử so sánh và logic trả về `1` hoặc `0`.

```c
if (5)       { /* luôn chạy */ }
if (0)       { /* không bao giờ chạy */ }
if (-3)      { /* chạy: -3 khác 0 */ }
int *p = NULL;
if (p)       { /* không chạy: NULL bằng 0 */ }
if (!p)      { /* chạy: p là NULL */ }
```

## 5.2. Câu lệnh `if` / `else`

### Cú pháp

```c
if (điều_kiện) {
    // chạy khi điều kiện đúng
} else if (điều_kiện_khác) {
    // chạy khi điều kiện đầu sai và điều kiện này đúng
} else {
    // chạy khi tất cả điều kiện trên đều sai
}
```

Các điều kiện được xét **từ trên xuống**, nhánh đầu tiên đúng sẽ chạy và những nhánh còn lại bị bỏ qua.

### Ví dụ: phân loại số

```c
// classify.c
#include <stdio.h>

int main(void) {
    int n;
    printf("Nhap mot so nguyen: ");
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Khong phai so nguyen\n");
        return 1;
    }

    if (n > 0) {
        printf("%d la so duong\n", n);
    } else if (n < 0) {
        printf("%d la so am\n", n);
    } else {
        printf("So 0\n");
    }

    printf("%d la so %s\n", n, (n % 2 == 0) ? "chan" : "le");
    return 0;
}
```

### Luôn dùng dấu ngoặc nhọn

C cho phép bỏ `{ }` khi chỉ có một câu lệnh, nhưng điều này rất dễ gây lỗi khi sửa mã:

```c
if (x > 0)
    printf("duong\n");
    printf("kiem tra xong\n");   // KHÔNG thuộc if dù thụt lề: luôn chạy!
```

Trường hợp còn nguy hiểm hơn là "dangling else" — `else` gắn với `if` **gần nhất**:

```c
if (a > 0)
    if (b > 0)
        printf("a, b duong\n");
else                              // else này thuộc về if (b > 0), không phải if (a > 0)!
    printf("a khong duong\n");
```

Nếu luôn dùng `{ }`, cả hai vấn đề này không xảy ra. Đó là quy tắc bắt buộc trong nhiều chuẩn mã hóa (MISRA, CERT C).

### Điều kiện phức hợp và đánh giá tắt

```c
if (age >= 18 && age <= 65) { ... }            // trong khoảng 18..65
if (c == 'y' || c == 'Y') { ... }               // một trong hai
if (!(x > 0)) { ... }                           // tương đương x <= 0
```

Vì `&&`/`||` đánh giá tắt, thứ tự có ý nghĩa: đặt điều kiện "bảo vệ" **trước**.

```c
if (i < n && arr[i] == target) { ... }   // đúng: chỉ đọc arr[i] khi i hợp lệ
if (arr[i] == target && i < n) { ... }   // SAI: có thể đọc ngoài mảng trước khi kiểm tra i
```

### Các lỗi thường gặp với `if`

```c
if (x = 5)          // gán thay vì so sánh; luôn đúng vì 5 khác 0
if (x == 5);        // dấu ; thừa: thân if rỗng
{ printf("..."); }  // khối này luôn chạy
if (0 < x < 10)     // KHÔNG như toán học: (0 < x) cho 0 hoặc 1, rồi so sánh với 10 -> luôn đúng
if (0 < x && x < 10) // đúng
```

### Cách viết `if` lồng nhau gọn hơn

Thay vì lồng sâu, dùng **thoát sớm (early return / guard clause)**:

```c
// Lồng sâu: khó đọc
int process(FILE *f, char *buf) {
    if (f != NULL) {
        if (buf != NULL) {
            // ... việc chính ...
            return 0;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

// Thoát sớm: phẳng và dễ đọc
int process2(FILE *f, char *buf) {
    if (f == NULL)   return -1;
    if (buf == NULL) return -1;
    // ... việc chính ...
    return 0;
}
```

## 5.3. Câu lệnh `switch`

`switch` chọn một trong nhiều nhánh dựa trên giá trị **nguyên** (kể cả `char`, `enum`) của một biểu thức.

```c
switch (biểu_thức) {
    case giá_trị_1:
        // ...
        break;
    case giá_trị_2:
    case giá_trị_3:        // nhiều giá trị dùng chung một nhánh
        // ...
        break;
    default:
        // không khớp case nào
        break;
}
```

Quy tắc:

- Biểu thức và mọi `case` phải là **kiểu nguyên** (không dùng được `double` hay chuỗi). Nhãn `case` phải là **hằng số lúc biên dịch** (`case 3:`, `case 'a':`, `case MAX:` với `MAX` là `enum`/`#define`, **không** phải biến `const int` trong C).
- Không có hai `case` trùng giá trị.
- `default` là tùy chọn, nhưng nên có để xử lý giá trị bất ngờ.

### Fall-through (rơi xuyên)

Khi chương trình vào một `case`, nó chạy **tiếp qua các case bên dưới** cho đến khi gặp `break` (hoặc `return`, hoặc hết `switch`). Đây là hành vi *mặc định* của C.

```c
int n = 2;
switch (n) {
    case 1: printf("mot\n");
    case 2: printf("hai\n");     // vào đây
    case 3: printf("ba\n");      // rơi xuống, vẫn chạy
    default: printf("khac\n");   // rơi xuống, vẫn chạy
}
// In ra: hai / ba / khac
```

Thiếu `break` là lỗi phổ biến. Nếu cố ý rơi xuyên, hãy ghi chú rõ (và với gcc 7+, dùng `__attribute__((fallthrough));` hoặc comment `/* fall through */` để tắt cảnh báo `-Wimplicit-fallthrough`).

Dùng fall-through **có chủ đích** để gom nhóm:

```c
// Đếm số ngày trong tháng
int days_in_month(int month, int year) {
    switch (month) {
        case 4: case 6: case 9: case 11:
            return 30;
        case 2: {
            int leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            return leap ? 29 : 28;
        }
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        default:
            return -1;             // tháng không hợp lệ
    }
}
```

Ghi chú: nếu cần khai báo biến bên trong một `case`, hãy bọc nhánh đó trong `{ }` như `case 2` ở trên (nhãn `case` không thể đứng ngay trước một khai báo và biến sẽ "tràn" sang các case sau).

### `switch` với `enum`

```c
enum Color { RED, GREEN, BLUE };

const char *color_name(enum Color c) {
    switch (c) {
        case RED:   return "do";
        case GREEN: return "xanh la";
        case BLUE:  return "xanh duong";
    }
    return "khong ro";
}
```

Nếu bạn liệt kê **không đủ** các giá trị `enum` và **không** có `default`, gcc với `-Wall` cảnh báo `-Wswitch` — rất hữu ích khi sau này bạn thêm một giá trị `enum` mới.

### Khi nào dùng `switch`, khi nào dùng `if-else`?

- Dùng `switch` khi so sánh **một biểu thức nguyên** với **nhiều hằng cụ thể** (menu, mã lệnh, trạng thái).
- Dùng `if-else` khi điều kiện là khoảng (`x > 10`), so sánh chuỗi, hoặc điều kiện phức hợp.

## 5.4. Vòng lặp `while`

```c
while (điều_kiện) {
    // thân vòng lặp
}
```

Kiểm tra điều kiện **trước** mỗi lần lặp. Nếu ngay từ đầu điều kiện sai, thân vòng lặp chạy **0 lần**.

```c
// Tính tổng các chữ số của n (n >= 0)
int digit_sum(int n) {
    int sum = 0;
    while (n > 0) {
        sum += n % 10;     // lấy chữ số hàng đơn vị
        n /= 10;           // bỏ chữ số đó
    }
    return sum;
}
// digit_sum(1234) = 4 + 3 + 2 + 1 = 10
```

Theo dõi từng bước với `n = 1234`:

| Lần lặp | `n` trước | `n % 10` | `sum` sau | `n` sau |
|---|---|---|---|---|
| 1 | 1234 | 4 | 4 | 123 |
| 2 | 123 | 3 | 7 | 12 |
| 3 | 12 | 2 | 9 | 1 |
| 4 | 1 | 1 | 10 | 0 |

`n` bằng 0 nên điều kiện `n > 0` sai và vòng lặp dừng. **Kỹ thuật "chạy tay" (trace)** như bảng trên là cách tốt nhất để hiểu và kiểm tra vòng lặp — hãy làm nó mỗi khi bạn viết một vòng lặp mới.

### Vòng lặp theo điều kiện chưa biết trước số lần

```c
// Thuật toán Euclid: ước chung lớn nhất
int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}
```

## 5.5. Vòng lặp `do-while`

```c
do {
    // thân vòng lặp
} while (điều_kiện);      // chú ý dấu ; ở cuối
```

Kiểm tra điều kiện **sau** mỗi lần lặp, nên thân chạy **ít nhất một lần**. Rất phù hợp cho việc "hỏi người dùng cho đến khi nhập đúng":

```c
// Hỏi cho đến khi người dùng nhập số trong 1..10
int n;
do {
    printf("Nhap so tu 1 den 10: ");
    if (scanf("%d", &n) != 1) {
        // xóa đầu vào lỗi, tránh lặp vô hạn
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        if (c == EOF) return 1;
        n = 0;                   // buộc hỏi lại
    }
} while (n < 1 || n > 10);
printf("Ban da nhap %d\n", n);
```

Đoạn xóa đầu vào ở trên **rất quan trọng**: nếu người dùng gõ chữ `abc`, `scanf` không đọc được và để lại `abc` trong bộ đệm; nếu không xóa, lần lặp sau `scanf` lại thất bại ngay và chương trình lặp vô hạn.

## 5.6. Vòng lặp `for`

```c
for (khởi_tạo; điều_kiện; cập_nhật) {
    // thân vòng lặp
}
```

Tương đương với:

```c
khởi_tạo;
while (điều_kiện) {
    // thân
    cập_nhật;
}
```

Thứ tự thực hiện: (1) `khởi_tạo` chạy **một lần**; (2) kiểm tra `điều_kiện`, sai thì thoát; (3) chạy thân; (4) chạy `cập_nhật`; quay lại bước 2.

```c
for (int i = 0; i < 5; i++) {
    printf("%d ", i);
}
// In: 0 1 2 3 4
```

`for` phù hợp khi **biết trước số lần lặp** (đi qua mảng, đếm). Từ C99 có thể khai báo biến trong phần khởi tạo (`int i = 0`), biến đó chỉ tồn tại trong vòng lặp.

### Quy ước "nửa mở" và lỗi lệch một

Với mảng `n` phần tử, chỉ số hợp lệ là `0 .. n-1`. Cách viết chuẩn: `for (i = 0; i < n; i++)`.

```c
int a[5];
for (int i = 0; i <= 5; i++) {   // SAI: i = 5 ghi ngoài mảng (off-by-one) -> UB
    a[i] = 0;
}
```

Lỗi *off-by-one* (lệch một) là một trong các lỗi phổ biến nhất. Hãy luôn tự hỏi ở lần lặp đầu và cuối, `i` bằng bao nhiêu và có truy cập ngoài giới hạn không.

### Biến thể của `for`

```c
for (int i = 10; i > 0; i--)         // đếm ngược
for (int i = 0; i < n; i += 2)       // bước nhảy 2
for (int i = 0, j = n - 1; i < j; i++, j--) {  // hai biến chạy từ hai đầu
    // hoán đổi a[i] và a[j] để đảo mảng
}
for (;;) { ... }                     // vòng lặp vô hạn (mọi phần đều được bỏ trống)
```

### Vòng lặp lồng nhau

```c
// Bảng cửu chương
for (int i = 1; i <= 9; i++) {
    for (int j = 1; j <= 10; j++) {
        printf("%d x %2d = %3d\n", i, j, i * j);
    }
    printf("\n");
}
```

Số lần chạy thân vòng trong là tích (9 × 10 = 90). Với vòng lặp lồng nhau `n` mức, độ phức tạp có thể tăng theo lũy thừa — cần lưu ý khi dữ liệu lớn.

Ví dụ vẽ tam giác:

```c
int n = 5;
for (int row = 1; row <= n; row++) {
    for (int col = 0; col < n - row; col++) putchar(' ');
    for (int col = 0; col < 2 * row - 1; col++) putchar('*');
    putchar('\n');
}
```

```text
    *
   ***
  *****
 *******
*********
```

## 5.7. `break`, `continue` và `goto`

### `break`

Thoát **ngay** khỏi vòng lặp (hoặc `switch`) gần nhất bên trong.

```c
// Tìm phần tử đầu tiên bằng target
int index = -1;
for (int i = 0; i < n; i++) {
    if (a[i] == target) {
        index = i;
        break;             // đã tìm thấy, không cần duyệt tiếp
    }
}
```

`break` chỉ thoát **một tầng**. Để thoát khỏi nhiều vòng lồng nhau, dùng cờ hoặc tách thành hàm và `return`:

```c
int find_pair(int a[], int n, int target, int *x, int *y) {
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (a[i] + a[j] == target) {
                *x = i; *y = j;
                return 1;          // thoát cả hai vòng
            }
        }
    }
    return 0;
}
```

### `continue`

Bỏ qua phần còn lại của lần lặp hiện tại và chuyển sang lần lặp tiếp theo (với `for`, bước `cập_nhật` vẫn chạy).

```c
// In các số lẻ từ 1 đến 10
for (int i = 1; i <= 10; i++) {
    if (i % 2 == 0) continue;     // bỏ qua số chẵn
    printf("%d ", i);
}
```

Cẩn thận khi dùng `continue` trong `while`: nếu bước cập nhật nằm **sau** `continue`, nó sẽ bị bỏ qua và gây vòng lặp vô hạn.

```c
int i = 0;
while (i < 10) {
    if (i % 2 == 0) continue;   // BUG: i không bao giờ tăng nếu i chẵn -> lặp vô hạn
    i++;
}
```

### `goto`

`goto nhãn;` nhảy đến `nhãn:` trong cùng hàm. Nói chung nên **tránh** vì làm luồng khó theo dõi. Tuy nhiên có một trường hợp được chấp nhận rộng rãi trong C: **dọn dẹp tài nguyên khi có lỗi** (vì C không có ngoại lệ).

```c
int do_work(void) {
    int rc = -1;
    char *buf = NULL;
    FILE *f = NULL;

    buf = malloc(1024);
    if (!buf) goto cleanup;

    f = fopen("data.txt", "r");
    if (!f) goto cleanup;

    // ... dùng buf và f ...
    rc = 0;

cleanup:
    if (f) fclose(f);
    free(buf);         // free(NULL) an toàn
    return rc;
}
```

Mẫu này (một điểm dọn dẹp duy nhất) sẽ xuất hiện lại ở chương 13.

## 5.8. Vòng lặp vô hạn và cách tránh

Một vòng lặp vô hạn xảy ra khi điều kiện không bao giờ trở thành sai. Một số **cố ý** (server chờ kết nối, vòng lặp game):

```c
while (1) {
    // xử lý sự kiện...
    if (should_quit) break;
}
```

Nhưng phần lớn là **vô ý**. Các nguyên nhân thường gặp:

1. **Quên cập nhật biến điều kiện:**
   ```c
   int i = 0;
   while (i < 10) { printf("%d\n", i); }   // thiếu i++
   ```
2. **Cập nhật sai hướng:** `for (int i = 10; i > 0; i++)`.
3. **Điều kiện luôn đúng với `unsigned`:**
   ```c
   for (unsigned int i = 5; i >= 0; i--) { ... }   // i >= 0 luôn đúng: 0 - 1 quay vòng thành 4294967295
   ```
   Sửa: dùng kiểu có dấu, hoặc viết `for (unsigned i = 5; i-- > 0; )` (mẹo "i tiến tới 0").
4. **So sánh số thực với `==`:** `for (double x = 0.0; x != 1.0; x += 0.1)` — vì 0.1 không biểu diễn chính xác nên `x` không bao giờ đúng bằng `1.0`. Dùng biến đếm nguyên: `for (int k = 0; k <= 10; k++) { double x = k / 10.0; ... }`.
5. **Đầu vào lỗi không được xử lý** (xem 5.5).

Để dừng chương trình đang chạy vô hạn trong terminal: nhấn **Ctrl+C**.

## 5.9. Đọc đến hết đầu vào

Mẫu rất thường gặp: đọc dữ liệu cho tới khi hết (EOF). Trên terminal, nhấn Ctrl+D (Linux/macOS) hoặc Ctrl+Z rồi Enter (Windows) để phát tín hiệu EOF; khi chuyển hướng file (`./prog < data.txt`), EOF tự đến cuối file.

```c
// sum_input.c — cộng tất cả số nguyên được nhập
#include <stdio.h>

int main(void) {
    long sum = 0;
    int x;
    int count = 0;

    while (scanf("%d", &x) == 1) {      // dừng khi không đọc được số nữa (EOF hoặc chữ)
        sum += x;
        count++;
    }

    if (count > 0) {
        printf("count = %d, sum = %ld, average = %.2f\n", count, sum, (double)sum / count);
    } else {
        printf("khong co so nao\n");
    }
    return 0;
}
```

Đọc từng ký tự:

```c
int c;                                   // phải là int, không phải char, để phân biệt được EOF (-1)
int lines = 0;
while ((c = getchar()) != EOF) {
    if (c == '\n') lines++;
}
```

## 5.10. Ví dụ thực hành đầy đủ

### Kiểm tra số nguyên tố

Ý tưởng: một số `n > 1` là nguyên tố nếu không có ước nào trong khoảng `2 .. √n`. Nếu `n = a × b` với `a ≤ b` thì `a ≤ √n`, nên chỉ cần thử đến `√n`.

```c
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
```

Điều kiện `i * i <= n` tránh dùng `sqrt` (tránh sai số số thực và không cần `-lm`). Với `n` rất lớn, `i * i` có thể tràn; ở mức bài tập này chấp nhận được.

### Menu tương tác

```c
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
```

Lưu ý: trong `do-while`, `continue` nhảy đến phần **kiểm tra điều kiện** (`choice != 0`), chứ không phải đầu thân vòng lặp. Vì đặt `choice = -1` trước `continue` nên vòng lặp tiếp tục.

## 5.11. Tóm tắt

- `0` là sai, khác 0 là đúng. Luôn dùng `{ }` cho thân `if`/vòng lặp.
- `switch` chỉ cho giá trị nguyên và hằng lúc biên dịch; nhớ `break`, ghi chú fall-through cố ý.
- `while` kiểm tra trước; `do-while` chạy ít nhất một lần; `for` dành cho số lần lặp biết trước.
- Cẩn thận với: off-by-one, `unsigned` với `>= 0`, `continue` bỏ qua bước cập nhật, đầu vào lỗi gây lặp vô hạn.
- Chạy tay (trace) vòng lặp để kiểm tra; dùng thoát sớm để tránh lồng `if` sâu.
- `goto cleanup` là mẫu dọn dẹp được chấp nhận trong C.

## 5.12. Bài tập

1. Viết chương trình in bảng cửu chương 1–9 (mỗi bảng 10 dòng) bằng vòng `for` lồng nhau, căn cột bằng `%2d`, `%3d`.
2. Viết `bool is_prime(int n)` và in tất cả số nguyên tố trong 2..100. Sau đó tìm số nguyên tố lớn nhất nhỏ hơn 1.000.000.
3. Viết menu có ba lựa chọn: [1] In bảng cửu chương, [2] Kiểm tra số nguyên tố, [0] Thoát. Xử lý được khi người dùng nhập chữ.
4. Viết chương trình đoán số: máy chọn ngẫu nhiên số 1..100 (`rand()`), người chơi đoán, chương trình báo "lớn hơn/nhỏ hơn" và đếm số lần đoán.
5. Viết chương trình in dãy Fibonacci nhỏ hơn 1000, và chương trình tính `n!` (kiểm tra tràn số ở `n` nào với `int`, với `unsigned long long`).
6. Viết chương trình đọc các số đến hết đầu vào, in ra số nhỏ nhất, lớn nhất và trung bình.
7. Viết chương trình in tam giác Pascal 8 dòng (gợi ý: `C(n,k) = C(n,k-1) × (n-k+1) / k`).
8. Tìm lỗi: đoạn mã sau định in các số chẵn từ 0 đến 10, nhưng lặp vô hạn. Giải thích và sửa.

```c
int i = 0;
while (i <= 10) {
    if (i % 2 != 0) continue;
    printf("%d\n", i);
    i++;
}
```

9. (Thử thách) Viết chương trình chuyển số nguyên thập phân sang nhị phân bằng vòng lặp chia 2, in kết quả đúng thứ tự.

Lời giải gợi ý: xem thư mục code và Lời giải ở cuối sách.

Mã nguồn mẫu: /code/chapter-05
