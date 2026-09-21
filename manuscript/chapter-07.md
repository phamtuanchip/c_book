# Chương 7 — Mảng & chuỗi (Arrays & Strings)

## Mục tiêu chương

- Hiểu mảng là **dãy phần tử liền kề trong bộ nhớ** và hệ quả của việc đó.
- Khai báo, khởi tạo, duyệt mảng một chiều và nhiều chiều; truyền mảng cho hàm.
- Hiểu chuỗi C là mảng `char` kết thúc bằng `'\0'`, và vì sao điều này gây ra rất nhiều lỗi.
- Dùng đúng và an toàn các hàm chuỗi: `strlen`, `strcpy`/`strncpy`, `snprintf`, `strcmp`, `strchr`, `strstr`, `strtok`.
- Nhận diện **tràn bộ đệm (buffer overflow)** và cách tránh.
- Biết giới hạn của chuỗi C với UTF-8 (tiếng Việt).

## 7.1. Mảng là gì?

**Mảng** là một dãy các phần tử **cùng kiểu**, nằm **liên tiếp nhau** trong bộ nhớ, truy cập bằng **chỉ số** bắt đầu từ **0**.

```c
int scores[5];          // mảng 5 số nguyên, chưa khởi tạo (giá trị rác nếu là biến cục bộ)
```

Trong bộ nhớ (giả sử `int` 4 byte, mảng bắt đầu ở địa chỉ `1000`):

```text
địa chỉ:   1000   1004   1008   1012   1016
         ┌──────┬──────┬──────┬──────┬──────┐
scores:  │  [0] │  [1] │  [2] │  [3] │  [4] │
         └──────┴──────┴──────┴──────┴──────┘
```

Phần tử `scores[i]` nằm ở địa chỉ `1000 + i × sizeof(int)`. Vì tính địa chỉ chỉ là một phép nhân và cộng, truy cập bất kỳ phần tử nào đều nhanh như nhau (**truy cập ngẫu nhiên O(1)**).

### Khai báo và khởi tạo

```c
int a[5];                       // không khởi tạo
int b[5] = {10, 20, 30, 40, 50};
int c[5] = {1, 2};              // phần còn lại được đặt 0: {1, 2, 0, 0, 0}
int d[5] = {0};                 // tất cả bằng 0 — cách chuẩn để "xóa" mảng
int e[] = {3, 1, 4, 1, 5};      // compiler tự đếm: 5 phần tử
int f[5] = { [2] = 7, [4] = 9 };  // C99: designated initializer -> {0, 0, 7, 0, 9}
```

Số phần tử của mảng trong lúc biên dịch:

```c
size_t n = sizeof(e) / sizeof(e[0]);   // = 5
```

Phép chia này chỉ đúng với **mảng thật sự**, không đúng với con trỏ hay tham số hàm (xem 7.3).

### Truy cập và duyệt mảng

```c
#include <stdio.h>

int main(void) {
    int a[5] = {10, 20, 30, 40, 50};
    size_t n = sizeof a / sizeof a[0];

    a[2] = 99;                       // gán phần tử thứ 3
    long sum = 0;
    for (size_t i = 0; i < n; i++) {
        printf("a[%zu] = %d\n", i, a[i]);
        sum += a[i];
    }
    printf("tong = %ld, trung binh = %.2f\n", sum, (double)sum / n);
    return 0;
}
```

### C không kiểm tra giới hạn mảng

Đây là điều quan trọng nhất của chương. Với `int a[5]`, chỉ số hợp lệ là `0..4`. Truy cập `a[5]`, `a[-1]` hay `a[100]` **biên dịch bình thường** nhưng là **hành vi không xác định**:

```c
int a[5] = {0};
a[5] = 1;       // ghi vào ô ngay SAU mảng: có thể phá biến khác, hoặc crash, hoặc "không sao" (tạm thời)
int x = a[10];  // đọc ô ngẫu nhiên
```

Vì sao nguy hiểm? Bộ nhớ ngoài mảng có thể chứa biến khác, địa chỉ trở về của hàm, hoặc dữ liệu quan trọng. Ghi đè chúng là cơ chế của nhiều lỗ hổng bảo mật (chương 18). Chương trình có thể vẫn "chạy đúng" hôm nay, rồi hỏng bất ngờ sau khi thêm một biến khác. **Bạn** phải bảo đảm chỉ số luôn hợp lệ.

Hãy phát hiện lỗi sớm bằng sanitizer:

```bash
gcc -std=c11 -g -fsanitize=address,undefined -o prog prog.c
./prog
# ==...== ERROR: AddressSanitizer: stack-buffer-overflow on address ...
```

### Các thuật toán cơ bản trên mảng

```c
// Tìm phần tử lớn nhất
int max_of(const int a[], size_t n) {
    int m = a[0];                    // tiền điều kiện: n >= 1
    for (size_t i = 1; i < n; i++)
        if (a[i] > m) m = a[i];
    return m;
}

// Tìm kiếm tuyến tính: trả về chỉ số, hoặc -1 nếu không thấy
int linear_search(const int a[], size_t n, int key) {
    for (size_t i = 0; i < n; i++)
        if (a[i] == key) return (int)i;
    return -1;
}

```

Đảo ngược mảng tại chỗ (hai chỉ số đi từ hai đầu vào giữa):

```c
void reverse(int a[], size_t n) {
    if (n < 2) return;
    size_t i = 0, j = n - 1;
    while (i < j) {
        int t = a[i]; a[i] = a[j]; a[j] = t;
        i++;
        j--;
    }
}
```

Tìm kiếm nhị phân trên mảng **đã sắp xếp** (O(log n)):

```c
int binary_search(const int a[], size_t n, int key) {
    size_t lo = 0, hi = n;                 // tìm trong nửa mở [lo, hi)
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;   // tránh tràn số so với (lo + hi) / 2
        if (a[mid] == key) return (int)mid;
        if (a[mid] < key)  lo = mid + 1;
        else               hi = mid;
    }
    return -1;
}
```

Sắp xếp chèn (insertion sort), đơn giản và tốt cho mảng nhỏ:

```c
void insertion_sort(int a[], size_t n) {
    for (size_t i = 1; i < n; i++) {
        int key = a[i];
        size_t j = i;
        while (j > 0 && a[j - 1] > key) {   // dịch các phần tử lớn hơn key sang phải
            a[j] = a[j - 1];
            j--;
        }
        a[j] = key;
    }
}
```

Trong thực tế, hãy dùng `qsort` (chương 6).

## 7.2. Mảng nhiều chiều

Mảng hai chiều là "mảng của các mảng". C lưu **theo hàng (row-major)**: hết hàng 0 rồi đến hàng 1...

```c
int m[3][4] = {
    { 1,  2,  3,  4},
    { 5,  6,  7,  8},
    { 9, 10, 11, 12}
};
```

```text
Bộ nhớ (liên tiếp): 1 2 3 4 | 5 6 7 8 | 9 10 11 12
                     hàng 0    hàng 1     hàng 2
```

Phần tử `m[i][j]` ở vị trí `i × 4 + j` tính từ đầu. Duyệt theo hàng (vòng ngoài là hàng, vòng trong là cột) khớp với bố cục bộ nhớ nên **nhanh hơn** do tận dụng cache:

```c
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
        printf("%3d ", m[i][j]);
    }
    printf("\n");
}
```

### Truyền mảng hai chiều cho hàm

Phải cho biết **số cột** (các chiều sau chiều đầu tiên), vì compiler cần nó để tính địa chỉ:

```c
void print_matrix(int rows, int cols, int m[rows][cols]) {   // C99: kích thước là tham số đứng trước
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) printf("%3d ", m[i][j]);
        printf("\n");
    }
}

// cách cũ: cột cố định
#define COLS 4
void print_matrix2(int m[][COLS], int rows);
```

Ví dụ: nhân hai ma trận `A (n×k)` và `B (k×p)`:

```c
void matmul(int n, int k, int p, const int A[n][k], const int B[k][p], int C[n][p]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < p; j++) {
            int s = 0;
            for (int t = 0; t < k; t++) s += A[i][t] * B[t][j];
            C[i][j] = s;
        }
    }
}
```

### Mảng độ dài biến đổi (VLA)

C99 cho phép kích thước mảng cục bộ là biến: `int a[n];`. Tiện, nhưng: nằm trên **stack** nên `n` lớn gây tràn stack, không kiểm tra được lỗi cấp phát, và bị đánh dấu *tùy chọn* từ C11. **Khuyến nghị:** dùng `malloc` (chương 9) cho kích thước không biết trước.

## 7.3. Mảng và tham số hàm

Khi truyền mảng cho hàm, mảng **phân rã (decay)** thành **con trỏ tới phần tử đầu tiên**. Hàm không biết độ dài mảng.

```c
#include <stdio.h>

void f(int a[]) {                     // tương đương với: void f(int *a)
    printf("trong f: sizeof(a) = %zu\n", sizeof a);    // kích thước CON TRỎ (8), không phải mảng
}

int main(void) {
    int a[10];
    printf("trong main: sizeof(a) = %zu\n", sizeof a); // 40 (10 × 4)
    f(a);
    return 0;
}
```

Hệ quả:

1. **Luôn truyền kèm kích thước** `(int a[], size_t n)`.
2. Hàm có thể **sửa mảng gốc** (vì nhận địa chỉ, không phải bản sao). Nếu không sửa, dùng `const`.
3. Mẹo `sizeof(a)/sizeof(a[0])` chỉ dùng được ở nơi khai báo mảng.

Mảng **không thể gán cho nhau** (`a = b;` là lỗi) và **không thể so sánh bằng `==`** (chỉ so sánh địa chỉ). Dùng `memcpy` để sao chép và `memcmp` để so sánh:

```c
#include <string.h>
int src[3] = {1, 2, 3}, dst[3];
memcpy(dst, src, sizeof src);           // sao chép 12 byte
if (memcmp(dst, src, sizeof src) == 0) printf("giong nhau\n");
```

## 7.4. Chuỗi trong C

C **không có kiểu chuỗi**. **Chuỗi** là **mảng `char` kết thúc bằng ký tự null `'\0'`** (giá trị 0). Ký tự `'\0'` đánh dấu nơi chuỗi kết thúc.

```c
char s[6] = "hello";     // 6 byte: 'h' 'e' 'l' 'l' 'o' '\0'
```

```text
chỉ số:   0    1    2    3    4    5
        ┌────┬────┬────┬────┬────┬────┐
   s:   │ 'h'│ 'e'│ 'l'│ 'l'│ 'o'│'\0'│
        └────┴────┴────┴────┴────┴────┘
        ←────── độ dài 5 ──────→ ↑ ký tự kết thúc
```

**Độ dài chuỗi** (`strlen`) là số ký tự **không tính** `'\0'`, nhưng mảng phải có chỗ cho nó: chuỗi dài 5 cần mảng ít nhất **6** phần tử. Quên chỗ cho `'\0'` là lỗi cực kỳ phổ biến.

### Các cách khai báo chuỗi

```c
char a[] = "hello";        // mảng 6 byte, được sao chép từ literal; SỬA ĐƯỢC
char b[10] = "hello";      // mảng 10 byte, phần còn lại là '\0'
char c[5] = "hello";       // LỖI/UB: không có chỗ cho '\0' (compiler cảnh báo)
const char *p = "hello";   // con trỏ tới chuỗi literal nằm trong vùng chỉ đọc; KHÔNG được sửa
char *q = "hello";         // hợp lệ nhưng nguy hiểm: q[0] = 'H'; là UB (thường crash)
```

**Khác biệt quan trọng:** `char a[] = "hello"` tạo **bản sao** trong mảng của bạn; còn `const char *p = "hello"` chỉ trỏ tới hằng chuỗi có sẵn trong chương trình. Luôn dùng `const char *` khi trỏ tới literal.

### Duyệt chuỗi

```c
// Tự cài đặt strlen
size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

// Đếm số chữ hoa
int count_upper(const char *s) {
    int c = 0;
    for (; *s; s++) {                   // *s là '\0' thì điều kiện sai -> dừng
        if (*s >= 'A' && *s <= 'Z') c++;
    }
    return c;
}
```

### Nhập chuỗi an toàn

```c
char line[100];
if (fgets(line, sizeof line, stdin) != NULL) {
    line[strcspn(line, "\n")] = '\0';    // xóa '\n' (nếu có): strcspn trả về vị trí của '\n' hoặc chiều dài chuỗi
}
```

`fgets` đọc tối đa `sizeof line - 1` ký tự và luôn thêm `'\0'`. Nếu dòng nhập dài hơn bộ đệm, phần còn lại **ở lại** trong stdin cho lần đọc sau — hãy kiểm tra xem ký tự `\n` có trong `line` không để biết dòng có bị cắt.

**Tuyệt đối không dùng `gets`** (đã bị xóa khỏi C11 vì không thể dùng an toàn). Với `scanf("%s", ...)` phải luôn có giới hạn độ rộng (`%99s`).

## 7.5. Thư viện `<string.h>`

### Độ dài và so sánh

```c
size_t strlen(const char *s);                       // độ dài (không tính '\0')
int    strcmp(const char *a, const char *b);        // <0, 0, >0 theo thứ tự từ điển
int    strncmp(const char *a, const char *b, size_t n);   // so sánh tối đa n ký tự
```

**Không** so sánh chuỗi bằng `==`: nó so sánh **địa chỉ**, không so sánh nội dung.

```c
char a[] = "abc", b[] = "abc";
if (a == b)            printf("cung dia chi\n");      // FALSE: hai mảng khác nhau
if (strcmp(a, b) == 0) printf("noi dung giong nhau\n"); // TRUE
```

### Sao chép và nối: nguy hiểm

```c
char *strcpy(char *dst, const char *src);          // KHÔNG kiểm tra kích thước dst
char *strcat(char *dst, const char *src);          // KHÔNG kiểm tra kích thước dst
```

```c
char small[8];
strcpy(small, "chuoi rat dai");     // TRÀN BỘ ĐỆM: ghi 14 byte vào mảng 8 byte -> UB
```

Chỉ dùng `strcpy`/`strcat` khi bạn **đã chứng minh** đích đủ chỗ. Thay thế phổ biến:

```c
char *strncpy(char *dst, const char *src, size_t n);
```

Nhưng `strncpy` có bẫy: nếu `src` dài ≥ `n` thì **không** thêm `'\0'`. Nếu dùng nó, phải tự thêm: `dst[n-1] = '\0'`. Cách được khuyến nghị nhất là **`snprintf`** — luôn thêm `'\0'` và không bao giờ ghi quá kích thước:

```c
char dst[16];
int len = snprintf(dst, sizeof dst, "%s-%d", "user", 42);   // "user-42"
if (len < 0 || (size_t)len >= sizeof dst) {
    // bị cắt ngắn: len là độ dài lẽ ra cần, có thể dùng để xử lý
}
```

Để nối chuỗi an toàn, dùng `snprintf` với offset hoặc tự viết hàm có kiểm tra:

```c
// Nối src vào dst (dst có kích thước cap); trả về 0 nếu vừa, -1 nếu bị cắt
int safe_append(char *dst, size_t cap, const char *src) {
    size_t used = strlen(dst);
    if (used >= cap) return -1;
    int n = snprintf(dst + used, cap - used, "%s", src);
    return (n >= 0 && (size_t)n < cap - used) ? 0 : -1;
}
```

Một số hệ thống (BSD, macOS, glibc ≥ 2.38) có `strlcpy`/`strlcat` an toàn hơn, nhưng chưa thuộc chuẩn C11.

### Tìm kiếm

```c
char *strchr(const char *s, int c);       // vị trí đầu tiên của ký tự c, hoặc NULL
char *strrchr(const char *s, int c);      // vị trí cuối cùng
char *strstr(const char *hay, const char *needle);  // vị trí chuỗi con đầu tiên, hoặc NULL
size_t strspn(const char *s, const char *accept);   // độ dài đoạn đầu chỉ gồm ký tự trong accept
size_t strcspn(const char *s, const char *reject);  // độ dài đoạn đầu KHÔNG chứa ký tự trong reject
```

```c
const char *s = "user@example.com";
const char *at = strchr(s, '@');
if (at != NULL) {
    printf("ten: %.*s\n", (int)(at - s), s);   // "user"  (%.*s in tối đa N ký tự)
    printf("mien: %s\n", at + 1);              // "example.com"
}
```

### Tách chuỗi: `strtok`

```c
char line[] = "an,binh,cuong";        // phải là mảng sửa được (strtok GHI '\0' vào chuỗi)
for (char *tok = strtok(line, ","); tok != NULL; tok = strtok(NULL, ",")) {
    printf("[%s]\n", tok);
}
```

Cảnh báo về `strtok`: nó **sửa chuỗi gốc**, dùng **trạng thái tĩnh nội bộ** (không an toàn với đa luồng, không lồng nhau được), và coi nhiều dấu phân cách liên tiếp như một. Phiên bản an toàn luồng: `strtok_r` (POSIX) / `strtok_s` (Windows).

### Chuyển chuỗi ↔ số

```c
#include <stdlib.h>
long   strtol(const char *s, char **end, int base);
double strtod(const char *s, char **end);
```

Dùng chúng thay cho `atoi`/`atof` (không báo lỗi). Xem `read_int` ở chương 4. Đổi số sang chuỗi: `snprintf(buf, sizeof buf, "%d", n)`.

### Bộ nhớ thô: `<string.h>` cũng có

```c
void *memcpy(void *dst, const void *src, size_t n);    // sao chép n byte, KHÔNG được chồng lấn
void *memmove(void *dst, const void *src, size_t n);   // như memcpy nhưng cho phép chồng lấn
void *memset(void *p, int byte, size_t n);             // gán n byte bằng giá trị byte
int   memcmp(const void *a, const void *b, size_t n);
```

`memset(arr, 0, sizeof arr)` xóa mảng; nhưng đừng dùng `memset` cho giá trị khác 0 với mảng `int` (nó gán theo từng byte).

## 7.6. Tràn bộ đệm — chi tiết

**Tràn bộ đệm (buffer overflow)** xảy ra khi ghi nhiều dữ liệu hơn kích thước vùng nhớ đã cấp. Đây là nguyên nhân của một phần rất lớn các lỗ hổng bảo mật lịch sử (ví dụ sâu Morris 1988 dùng `gets`).

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char name[8];
    int is_admin = 0;                 // biến kề bên trên stack (ví dụ)

    gets(name);                       // KHÔNG BAO GIỜ dùng gets
    // hoặc: strcpy(name, argv[1]);
    if (is_admin) printf("ban la admin!\n");
    return 0;
}
```

Nếu người dùng nhập chuỗi dài, các byte thừa có thể ghi đè `is_admin` (tùy cách compiler sắp xếp biến) hoặc đè lên địa chỉ trở về của hàm để chiếm quyền điều khiển chương trình.

### Phòng tránh — danh sách kiểm tra

1. Luôn biết **kích thước đích** và truyền nó (`sizeof buf`).
2. Dùng `fgets`, `snprintf`, `strncmp`, `memcpy` với độ dài đã kiểm tra; tránh `gets`, `strcpy`, `strcat`, `sprintf`.
3. Kiểm tra độ dài đầu vào **trước** khi sao chép.
4. Bật cảnh báo: `-Wall -Wextra -Wformat-security -D_FORTIFY_SOURCE=2 -O2` và bảo vệ stack `-fstack-protector-strong`.
5. Dùng sanitizer khi phát triển và kiểm thử: `-fsanitize=address`.

## 7.7. Chuỗi và Unicode (UTF-8)

Nếu chương trình đọc tiếng Việt (UTF-8):

- Mỗi ký tự ASCII chiếm 1 byte; chữ có dấu chiếm 2–3 byte; emoji 4 byte.
- `strlen("Việt")` = **6** (V=1, i=1, ệ=3, t=1) chứ không phải 4. `strlen` đếm **byte**.
- Cắt chuỗi theo byte có thể **làm hỏng** ký tự (cắt giữa chuỗi byte của `ệ`).
- `toupper`, `isalpha` của `<ctype.h>` chỉ đúng với ASCII, không xử lý chữ có dấu.

Nếu cần đếm ký tự UTF-8, ta đếm các byte **không phải byte tiếp nối** (byte tiếp nối có dạng `10xxxxxx`):

```c
size_t utf8_length(const char *s) {
    size_t count = 0;
    for (; *s; s++) {
        if (((unsigned char)*s & 0xC0) != 0x80) count++;   // đếm byte đầu của mỗi ký tự
    }
    return count;
}
// utf8_length("Việt") = 4
```

Với xử lý văn bản Unicode nghiêm túc (chuẩn hóa, so sánh, hiển thị độ rộng), hãy dùng thư viện như ICU hoặc utf8proc.

## 7.8. Ví dụ hoàn chỉnh: đếm từ và tần suất chữ cái

```c
// text_stats.c
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main(void) {
    char line[1024];
    int freq[26] = {0};       // freq[0] ứng với 'a', ..., freq[25] ứng với 'z'
    int words = 0, chars = 0;

    while (fgets(line, sizeof line, stdin) != NULL) {
        int in_word = 0;
        for (size_t i = 0; line[i] != '\0'; i++) {
            unsigned char c = (unsigned char)line[i];
            chars++;
            if (isalpha(c)) {
                freq[tolower(c) - 'a']++;
            }
            if (isspace(c)) {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                words++;          // bắt đầu một từ mới
            }
        }
    }

    printf("So ky tu: %d, so tu: %d\n", chars, words);
    for (int i = 0; i < 26; i++) {
        if (freq[i] > 0) printf("%c: %d\n", 'a' + i, freq[i]);
    }
    return 0;
}
```

Chạy: `echo "hello world hello" | ./text_stats`.

Kỹ thuật `freq[tolower(c) - 'a']` dùng ký tự làm **chỉ số mảng** (nhờ mã ký tự là số) là mẹo đếm tần suất rất phổ biến. Vì `isalpha` chỉ nhận ký tự ASCII trong locale "C", chỉ số luôn nằm trong `0..25`.

## 7.9. Tóm tắt

- Mảng là dãy phần tử cùng kiểu, liền kề, chỉ số từ 0; **C không kiểm tra giới hạn**.
- Truyền mảng cho hàm thực chất truyền con trỏ; luôn truyền thêm kích thước; `sizeof` của tham số mảng là kích thước con trỏ.
- Chuỗi = mảng `char` kết thúc bằng `'\0'`; chừa chỗ cho `'\0'`; đừng sửa chuỗi literal.
- So sánh chuỗi bằng `strcmp`, không phải `==`; sao chép chuỗi bằng `snprintf` hoặc `memcpy` có kiểm tra độ dài.
- Không dùng `gets`; hạn chế `strcpy`, `strcat`, `sprintf`; dùng sanitizer để bắt tràn bộ đệm.
- `strlen` đếm byte, không đếm ký tự Unicode.

## 7.10. Bài tập

1. Viết hàm tính trung bình, tìm max/min và đếm số phần tử lớn hơn trung bình của một mảng số thực.
2. Viết hàm xoay mảng sang trái `k` vị trí (gợi ý: đảo ba lần).
3. Viết hàm hợp nhất hai mảng đã sắp xếp thành một mảng đã sắp xếp (O(n + m)).
4. Viết `my_strlen`, `my_strcpy`, `my_strcmp`, `my_strcat` (phiên bản có kiểm tra kích thước đích).
5. Viết hàm đảo ngược chuỗi tại chỗ, và hàm kiểm tra chuỗi đối xứng (bỏ qua khoảng trắng và chữ hoa/thường).
6. Viết hàm `trim` xóa khoảng trắng đầu và cuối một chuỗi (sửa tại chỗ).
7. Viết hàm đếm số lần xuất hiện của một chuỗi con trong chuỗi (không chồng lấn và có chồng lấn).
8. Đọc một dòng chứa các số cách nhau bằng dấu cách (ví dụ `"10 20 30"`), dùng `strtol` phân tích thành mảng `int`.
9. Viết chương trình xoay ma trận vuông `n×n` 90 độ theo chiều kim đồng hồ.
10. (Thử thách) Viết chương trình cố tình tràn bộ đệm trên stack rồi chạy với `-fsanitize=address`; đọc báo cáo và giải thích từng phần (loại lỗi, dòng gây lỗi, vùng nhớ bị ảnh hưởng).

Mã nguồn mẫu: /code/chapter-07
