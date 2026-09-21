# Chương 3 — Lịch sử và triết lý của ngôn ngữ C

## Mục tiêu chương

- Biết C ra đời trong bối cảnh nào và vì sao nó được thiết kế như vậy.
- Phân biệt các chuẩn C: K&R, C89/C90, C99, C11, C17, C23 và biết mỗi chuẩn thêm gì.
- Chọn đúng cờ `-std` khi biên dịch và hiểu vì sao nó quan trọng.
- Hiểu **hành vi không xác định (undefined behavior)**, **hành vi do triển khai (implementation-defined)** và **hành vi chưa chỉ định (unspecified)** — những khái niệm khó nhất và quan trọng nhất của C.
- Nắm triết lý "tin tưởng lập trình viên" và hệ quả thực tế của nó.

Chương này ít mã hơn các chương khác, nhưng những khái niệm về hành vi không xác định sẽ giải thích **vì sao** những lỗi kỳ lạ trong C xảy ra.

## 3.1. Lịch sử ngắn gọn

### Bối cảnh: từ assembly đến ngôn ngữ cấp cao

Cuối thập niên 1960, hệ điều hành thường được viết bằng **assembly** — nhanh nhưng gắn chặt với một loại CPU, mỗi lần đổi máy phải viết lại. Ken Thompson và Dennis Ritchie tại Bell Labs muốn viết UNIX theo cách **có thể chuyển sang máy khác**.

```text
1966  BCPL   (Martin Richards)   — ngôn ngữ không có kiểu dữ liệu
1969  B      (Ken Thompson)      — rút gọn từ BCPL, dùng cho UNIX đầu tiên trên PDP-7
1972  C      (Dennis Ritchie)    — thêm kiểu dữ liệu (int, char, con trỏ, struct) vào B
1973  Nhân UNIX được viết lại phần lớn bằng C — một bước ngoặt
1978  K&R: "The C Programming Language" xuất bản — định nghĩa "C của K&R"
1989  ANSI C (C89), chuẩn hóa lần đầu; 1990 thành ISO C90
1999  C99
2011  C11
2018  C17 (bản sửa lỗi của C11, không thêm tính năng mới)
2024  C23
```

Điểm mấu chốt: C ra đời từ nhu cầu thực tế — viết một hệ điều hành — chứ không phải từ lý thuyết. Đó là lý do nó gọn, thực dụng và "gần máy".

### Vì sao C tồn tại lâu đến vậy?

- **Di động:** chỉ cần có compiler C cho một CPU mới là chạy được phần lớn phần mềm.
- **Hiệu năng:** ít chi phí ẩn; những gì bạn viết gần như tương ứng trực tiếp với lệnh máy.
- **Nhỏ gọn:** ngôn ngữ có ít từ khóa; có thể nắm toàn bộ trong vài tuần.
- **Hệ sinh thái:** hệ điều hành, thư viện, driver hiện có đều viết bằng C nên mọi ngôn ngữ khác đều cần "nói chuyện" được với C (qua *C ABI* / *FFI*).

## 3.2. Các phiên bản chuẩn của C

| Chuẩn | Cờ gcc | Tính năng đáng chú ý |
|---|---|---|
| K&R C (1978) | `-traditional` (cũ) | Khai báo hàm kiểu cũ, không có prototype |
| **C89/C90** (ANSI) | `-std=c89` | Prototype hàm, `void`, `const`, `volatile`, thư viện chuẩn |
| **C99** | `-std=c99` | Comment `//`, khai báo biến ở giữa khối lệnh, `for (int i...)`, `long long`, `<stdint.h>`, `<stdbool.h>`, `inline`, `restrict`, mảng độ dài biến đổi (VLA), designated initializers, `snprintf` |
| **C11** | `-std=c11` | `_Atomic`, `<threads.h>`, `_Generic`, `_Static_assert`, `_Alignof`, ký tự Unicode (`char16_t`, `char32_t`), loại bỏ `gets` |
| **C17/C18** | `-std=c17` | Chỉ sửa lỗi kỹ thuật của C11 |
| **C23** | `-std=c2x` / `c23` | `nullptr`, `bool`/`true`/`false` là từ khóa, `constexpr`, `#embed`, `typeof`, số nhị phân `0b...`, dấu nháy tách số `1'000'000` |

### Ví dụ: cùng một ý, khác chuẩn

```c
/* Kiểu C89: khai báo biến phải ở đầu khối, không có // */
int main(void) {
    int i;
    int sum = 0;
    for (i = 0; i < 5; i++) {
        sum += i;
    }
    return sum;
}
```

```c
// Kiểu C99 trở lên: khai báo biến ngay chỗ dùng, phạm vi hẹp hơn
#include <stdbool.h>
#include <stdint.h>

int main(void) {
    int32_t sum = 0;
    bool found = false;
    for (int i = 0; i < 5; i++) {   // i chỉ tồn tại trong vòng for
        sum += i;
        if (i == 3) found = true;
    }
    return found ? sum : 0;
}
```

**Khuyến nghị của sách:** dùng **C11** (`-std=c11`). Nó đủ hiện đại, được mọi compiler chính hỗ trợ. Đoạn mã C89 phía trên vẫn biên dịch được với `-std=c11` (C là gần như tương thích ngược), nhưng phong cách C99 rõ ràng hơn.

### Thử nghiệm với các chuẩn khác nhau

```c
// compat_test.c
#include <stdio.h>

int main(void) {
    // "long long" và "//" comment: C99 trở lên
    long long big = 9000000000LL;
    printf("big = %lld\n", big);

    // Macro cho biết chuẩn mà compiler đang dùng
    #ifdef __STDC_VERSION__
        printf("__STDC_VERSION__ = %ld\n", __STDC_VERSION__);
    #else
        printf("C89/C90 (khong co __STDC_VERSION__)\n");
    #endif
    return 0;
}
```

Biên dịch với từng chuẩn và quan sát:

```bash
gcc -std=c89 -pedantic -Wall compat_test.c -o t89    # cảnh báo: dùng "long long" và "//" không có trong C89
gcc -std=c99 -Wall compat_test.c -o t99              # __STDC_VERSION__ = 199901
gcc -std=c11 -Wall compat_test.c -o t11              # __STDC_VERSION__ = 201112
gcc -std=c17 -Wall compat_test.c -o t17              # __STDC_VERSION__ = 201710
```

Cờ `-pedantic` yêu cầu compiler cảnh báo mọi thứ **không đúng chuẩn** đã chọn; nếu không có nó, gcc dùng nhiều "mở rộng GNU" nên lẫn lộn giữa chuẩn và mở rộng. Cờ `-pedantic-errors` biến các cảnh báo đó thành lỗi.

### Mở rộng của compiler

GCC/Clang có thêm nhiều tính năng ngoài chuẩn (`__attribute__`, `typeof`, statement expressions...). Chúng tiện nhưng **làm mã không di động** sang MSVC. Nếu cần chạy trên nhiều compiler, hãy giữ trong chuẩn và dùng `-std=c11 -pedantic`. (Ghi chú: `-std=gnu11` là chuẩn C11 kèm mở rộng GNU, và là mặc định của gcc.)

## 3.3. Triết lý thiết kế của C

Dennis Ritchie và cộng sự thiết kế C với vài nguyên tắc thường được tóm tắt là "*spirit of C*":

1. **Tin tưởng lập trình viên (trust the programmer).** C giả định bạn biết mình làm gì. Nó không chặn bạn ghi ra ngoài mảng hay ép kiểu tùy ý.
2. **Không ngăn lập trình viên làm điều cần thiết.** Nếu việc đó cần thiết cho hệ thống (như truy cập trực tiếp bộ nhớ), C cho phép.
3. **Giữ ngôn ngữ nhỏ và đơn giản.**
4. **Chỉ có một cách để làm một việc.** (Mục tiêu, dù không phải lúc nào cũng đạt.)
5. **Làm cho nó nhanh, ngay cả khi không được dễ chuyển sang máy khác.** Nói cách khác: *không trả chi phí cho thứ bạn không dùng* (no hidden cost).

### Hệ quả thực tế

| Ngôn ngữ an toàn (Python, Java) | C |
|---|---|
| Truy cập `a[10]` trong mảng 5 phần tử → ném ngoại lệ | Đọc/ghi vào bộ nhớ nào đó bên cạnh mảng, **không báo lỗi** |
| Bộ nhớ được thu gom tự động | Bạn tự `malloc`/`free` |
| Kiểu dữ liệu kiểm tra khi chạy | Không kiểm tra khi chạy; ép kiểu tùy ý |
| Chương trình sai → dừng với thông báo rõ | Chương trình sai → có thể *có vẻ* chạy bình thường, sau đó hỏng ở nơi khác |

Đổi lại, C cho hiệu năng cao và khả năng kiểm soát tuyệt đối. Kỹ năng cần rèn: **tự đóng vai người kiểm tra** — mỗi dòng bạn viết, hãy hỏi "nếu đầu vào sai/quá lớn/NULL thì sao?".

## 3.4. Ba loại "hành vi không chắc chắn"

Chuẩn C phân biệt ba khái niệm dễ nhầm. Chúng cực kỳ quan trọng nên cần hiểu kỹ.

### 1. Implementation-defined behavior (do triển khai quy định)

Chuẩn cho phép nhiều lựa chọn, nhưng **mỗi compiler phải chọn một và ghi tài liệu**. Chương trình vẫn hợp lệ, nhưng kết quả có thể khác nhau giữa các compiler/nền tảng.

Ví dụ: `sizeof(int)`, `char` có dấu hay không, kết quả dịch phải số nguyên âm (`-8 >> 1`), kích thước `long`.

### 2. Unspecified behavior (chưa chỉ định)

Chuẩn cho vài khả năng hợp lệ nhưng **không bắt compiler ghi lại lựa chọn**, thậm chí nó có thể đổi giữa các lần biên dịch.

Ví dụ kinh điển: **thứ tự đánh giá các tham số của hàm**.

```c
#include <stdio.h>

int f(void) { printf("f "); return 1; }
int g(void) { printf("g "); return 2; }

int main(void) {
    printf("ket qua = %d\n", f() + g());
    // có thể in "f g" hoặc "g f" — chuẩn không quy định thứ tự
    return 0;
}
```

### 3. Undefined behavior (UB — hành vi không xác định)

Chuẩn **không đặt bất kỳ yêu cầu nào** khi UB xảy ra. Chương trình có thể: chạy đúng, in kết quả sai, crash, treo, hoặc — nguy hiểm nhất — *có vẻ đúng hôm nay và sai khi đổi compiler hoặc bật tối ưu hóa*.

Các nguồn UB phổ biến:

| Nguồn UB | Ví dụ |
|---|---|
| Tràn số nguyên **có dấu** | `INT_MAX + 1` |
| Truy cập ngoài giới hạn mảng | `int a[3]; a[5] = 1;` |
| Dereference con trỏ `NULL` hoặc con trỏ không hợp lệ | `int *p = NULL; *p = 1;` |
| Dùng bộ nhớ đã `free` (*use-after-free*) | `free(p); *p = 1;` |
| `free` hai lần (*double free*) | `free(p); free(p);` |
| Đọc biến cục bộ chưa khởi tạo | `int x; printf("%d", x);` |
| Chia cho 0 (số nguyên) | `1 / 0` |
| Dịch bit với số lượng ≥ số bit của kiểu hoặc âm | `1 << 32` với `int` 32 bit |
| Sửa một biến hai lần giữa hai *sequence point* | `i = i++ + ++i;` |
| Ghi vào chuỗi literal | `char *s = "abc"; s[0] = 'x';` |
| Truyền sai định dạng cho `printf` | `printf("%d", 3.14);` |

### Vì sao UB nguy hiểm? Ví dụ về tối ưu hóa

Compiler giả định rằng **UB không bao giờ xảy ra**, và dùng giả định đó để tối ưu.

```c
// ub_demo.c
#include <stdio.h>
#include <limits.h>

int is_next_bigger(int x) {
    return x + 1 > x;      // với số có dấu, tràn số là UB
}

int main(void) {
    printf("%d\n", is_next_bigger(INT_MAX));
    return 0;
}
```

- Biên dịch `gcc -O0`: thường in `0` (vì `INT_MAX + 1` quay vòng thành số âm).
- Biên dịch `gcc -O2`: thường in `1`, vì compiler lập luận "`x + 1` luôn lớn hơn `x` vì tràn số không xảy ra" và thay cả biểu thức bằng hằng `1`.

Cùng một mã nguồn, hai kết quả khác nhau. Đây là lý do UB được coi là lỗi nghiêm trọng ngay cả khi "chạy thử thấy đúng".

### Cách chống UB

1. **Bật cảnh báo:** `-Wall -Wextra`.
2. **Dùng sanitizer** (rất hiệu quả, sẽ học ở chương 9 và 18):

```bash
gcc -std=c11 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer ub_demo.c -o ub_demo
./ub_demo
```

UBSan sẽ báo `runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'`.

3. **Kiểm tra trước khi thao tác:** chỉ số mảng, con trỏ NULL, tràn số.
4. **Viết code đơn giản, dễ kiểm chứng.**

## 3.5. Viết mã di động

Để mã chạy được trên nhiều nền tảng:

1. **Đừng giả định kích thước kiểu.** Dùng `int32_t`, `uint64_t`, `size_t`.
2. **Đừng giả định endianness** khi ghi dữ liệu nhị phân ra file/mạng; hãy tuần tự hóa từng byte theo thứ tự cố định.
3. **Đừng giả định `char` có dấu hay không.** Dùng `unsigned char` khi làm việc với byte thô.
4. **Dùng `size_t` và `%zu`** cho kích thước; `PRId64` (từ `<inttypes.h>`) để in `int64_t`.
5. **Dùng macro của compiler để tách mã theo nền tảng** khi bắt buộc:

```c
#if defined(_WIN32)
    #include <windows.h>
    #define PLATFORM "Windows"
#elif defined(__linux__)
    #include <unistd.h>
    #define PLATFORM "Linux"
#elif defined(__APPLE__)
    #define PLATFORM "macOS"
#else
    #define PLATFORM "khong ro"
#endif
```

6. **Biên dịch bằng nhiều compiler** (gcc, clang) và nhiều mức cảnh báo — mỗi compiler phát hiện được các lỗi khác nhau.

Ví dụ in số 64 bit di động:

```c
#include <inttypes.h>
#include <stdio.h>

int main(void) {
    int64_t big = 9000000000LL;
    printf("big = %" PRId64 "\n", big);     // PRId64 mở rộng thành "ld" hoặc "lld" tùy nền tảng
    return 0;
}
```

## 3.6. Lời khuyên cho người mới

- **Luôn đọc cảnh báo** và sửa hết. Cảnh báo thường chỉ ra UB tiềm ẩn.
- **Chọn một chuẩn và ghi rõ** trong Makefile (`-std=c11`). Đừng để mặc định.
- **Không tin "chạy được trên máy tôi".** UB có thể chạy đúng ở máy bạn và sai ở máy khác.
- **Đọc chuẩn hoặc cppreference** khi nghi ngờ. `https://en.cppreference.com/w/c/language/behavior` là nơi bắt đầu tốt cho UB.
- **Học sanitizer sớm.** Nó biến những lỗi bộ nhớ "khó thấy" thành thông báo rõ ràng.

## 3.7. Tóm tắt

- C ra đời từ nhu cầu viết UNIX di động; triết lý là hiệu năng, đơn giản, và tin tưởng lập trình viên.
- Các chuẩn: C89 → C99 → C11 → C17 → C23. Sách dùng `-std=c11`.
- Có ba dạng hành vi "không chắc chắn": *implementation-defined*, *unspecified*, *undefined*. UB là nguy hiểm nhất vì compiler có quyền giả định nó không xảy ra.
- Viết mã di động bằng kiểu có kích thước cố định, tránh giả định nền tảng.
- Dùng cảnh báo và sanitizer để phát hiện UB sớm.

## 3.8. Bài tập

1. Biên dịch `compat_test.c` với `-std=c89 -pedantic`, `-std=c99`, `-std=c11`. Ghi lại từng thông báo và giải thích tại sao có khác biệt.
2. Chạy `ub_demo.c` với `-O0` và `-O2`. Ghi kết quả. Sau đó thêm `-fsanitize=undefined` và đọc thông báo lỗi.
3. Với mỗi nguồn UB trong bảng ở mục 3.4, tự viết một đoạn mã ngắn (khoảng 3–5 dòng) gây ra nó và chạy với `-fsanitize=address,undefined` xem sanitizer báo gì. (Thực hiện trong thư mục thử nghiệm riêng.)
4. Phân loại các hành vi sau là *implementation-defined*, *unspecified* hay *undefined*: (a) `sizeof(long)`; (b) `a[i] = i++;` (c) `int x = 5 / 0;` (d) thứ tự đánh giá `f() + g()`; (e) `-7 / 2`, biết C99 quy định làm tròn về 0.
5. Viết chương trình in tên hệ điều hành đang chạy dựa trên macro `_WIN32`, `__linux__`, `__APPLE__`.
6. Viết chương trình in `INT_MAX`, `INT_MIN`, `LONG_MAX`, `SIZE_MAX` và giải thích vì sao `LONG_MAX` khác nhau giữa Windows và Linux.

## 3.9. Tài nguyên

- Kernighan & Ritchie, *The C Programming Language*, 2nd ed.
- Dennis Ritchie, *The Development of the C Language* (bài viết lịch sử, tìm trên mạng).
- https://en.cppreference.com/w/c — tra cứu chuẩn.
- https://gcc.gnu.org/onlinedocs/gcc/Standards.html — chuẩn C được gcc hỗ trợ.
- John Regehr, *A Guide to Undefined Behavior in C and C++* (blog) — giải thích rất rõ vì sao UB nguy hiểm.

Mã nguồn mẫu: /code/chapter-03
