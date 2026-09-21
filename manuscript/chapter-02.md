# Chương 2 — Máy tính và lập trình cơ bản

## Mục tiêu chương

- Hiểu máy tính lưu trữ mọi thứ (số, chữ, hình ảnh) bằng **bit** và **byte** như thế nào.
- Đổi qua lại giữa hệ thập phân, nhị phân và thập lục phân.
- Hiểu cách biểu diễn số nguyên âm (bù hai), số thực và ký tự.
- Hiểu **endianness** và kiểm chứng nó bằng chương trình.
- Hình dung bộ nhớ của một chương trình: **stack**, **heap**, vùng dữ liệu, vùng mã.
- Biết `sizeof` trả về gì và vì sao kích thước kiểu dữ liệu phụ thuộc nền tảng.

Nếu chương 1 dạy bạn *gõ* chương trình, chương này dạy bạn *máy tính nhìn chương trình đó ra sao*. Những khái niệm ở đây là nền của toàn bộ phần con trỏ và bộ nhớ về sau.

## 2.1. Bit, byte và hệ nhị phân

### Bit và byte

- **Bit** (*binary digit*) là đơn vị thông tin nhỏ nhất, chỉ có hai giá trị `0` hoặc `1`. Phần cứng dùng hai mức điện áp để biểu diễn chúng.
- **Byte** là nhóm **8 bit**. Byte là đơn vị nhỏ nhất mà chương trình C có thể đánh địa chỉ.
- Một byte biểu diễn được 2⁸ = **256** giá trị khác nhau (từ 0 đến 255 nếu không có dấu).

Các đơn vị lớn hơn:

| Đơn vị | Số byte |
|---|---|
| 1 KB (kilobyte) | 1.000 byte (hoặc 1 KiB = 1.024) |
| 1 MB | 1.000.000 byte (hoặc 1 MiB = 1.048.576) |
| 1 GB | 1 tỷ byte (hoặc 1 GiB = 2³⁰ byte) |

### Hệ nhị phân hoạt động thế nào

Hệ thập phân dùng 10 chữ số; mỗi vị trí có "trọng số" là lũy thừa của 10. Hệ nhị phân làm y hệt, nhưng với 2:

```text
Số nhị phân 1011 gồm các bit, từ phải sang trái có trọng số 1, 2, 4, 8:

   bit:     1     0     1     1
   trọng số: 8     4     2     1
            ────  ────  ────  ────
   giá trị: 8  +  0  +  2  +  1  = 11 (thập phân)
```

**Đổi thập phân sang nhị phân** bằng cách chia liên tiếp cho 2 và lấy phần dư, đọc từ dưới lên. Ví dụ đổi 13:

```text
13 / 2 = 6  dư 1   ↑
 6 / 2 = 3  dư 0   │  đọc từ dưới lên
 3 / 2 = 1  dư 1   │
 1 / 2 = 0  dư 1   │
Kết quả: 1101
```

Kiểm tra: 8 + 4 + 0 + 1 = 13.

### Hệ thập lục phân (hexadecimal)

Số nhị phân dài rất khó đọc. Vì **4 bit = đúng 1 chữ số hex** nên lập trình viên dùng hệ 16 (chữ số `0–9` và `A–F`, với `A`=10, ..., `F`=15) để viết gọn:

```text
Nhị phân:   1111 0000 1010 0101
Hex:           F    0    A    5      →  0xF0A5
```

Trong C, số hex viết với tiền tố `0x` (`0xFF` = 255), số nhị phân dùng `0b` là mở rộng của compiler (`0b1010`, chưa phải chuẩn C11, chuẩn hóa ở C23), số bát phân viết với tiền tố `0` (`017` = 15).

> **Cẩn thận:** `int x = 010;` là số **8**, không phải 10, vì số bắt đầu bằng `0` là hệ bát phân. Đây là nguồn lỗi thú vị khi bạn định "căn số cho thẳng hàng".

### Chương trình in số ở nhiều hệ

```c
// bases.c
#include <stdio.h>

int main(void) {
    int n = 255;
    printf("thap phan: %d\n", n);
    printf("thap luc : %x (viet hoa: %X, kem tien to: %#x)\n", n, n, n);
    printf("bat phan : %o\n", n);
    return 0;
}
```

Kết quả: `255`, `ff (FF, 0xff)`, `377`.

## 2.2. Số nguyên có dấu và bù hai

Với **số nguyên không dấu (unsigned)** n bit, các giá trị từ 0 đến 2ⁿ − 1. Với 8 bit là 0..255.

Với **số nguyên có dấu (signed)**, gần như mọi máy tính hiện nay dùng phương pháp **bù hai (two's complement)**. Bit cao nhất là **bit dấu**: `0` là số không âm, `1` là số âm.

Cách tính giá trị: bit dấu có trọng số **âm**. Ví dụ với 8 bit, trọng số từ trái sang phải là −128, 64, 32, 16, 8, 4, 2, 1.

```text
0000 0101  =  4 + 1                          =    5
1111 1011  = -128 + 64+32+16+8 + 2 + 1       =   -5
1000 0000  = -128                            = -128  (giá trị nhỏ nhất)
0111 1111  = 64+32+16+8+4+2+1                =  127  (giá trị lớn nhất)
1111 1111  = -128 + 127                      =   -1
```

**Cách đổi nhanh một số dương sang số âm tương ứng:** *đảo tất cả các bit rồi cộng 1*.

```text
   5 =  0000 0101
đảo bit  1111 1010
cộng 1   1111 1011  =  -5
```

Vì sao dùng bù hai? Vì phép cộng phần cứng dùng **chung một mạch** cho cả số âm và số dương: `5 + (-5)` cho `0000 0101 + 1111 1011 = 1 0000 0000`, bit thứ 9 bị bỏ đi, còn `0000 0000` = 0.

### Khoảng giá trị

| Số bit | Có dấu | Không dấu |
|---|---|---|
| 8 | −128 … 127 | 0 … 255 |
| 16 | −32.768 … 32.767 | 0 … 65.535 |
| 32 | −2.147.483.648 … 2.147.483.647 | 0 … 4.294.967.295 |
| 64 | −9,22×10¹⁸ … 9,22×10¹⁸ | 0 … 1,84×10¹⁹ |

### Tràn số (overflow)

Điều gì xảy ra khi vượt khoảng giá trị?

- Với **số không dấu**, C định nghĩa rõ: giá trị "quay vòng" (modulo 2ⁿ). `255 + 1` trong `unsigned char` là `0`.
- Với **số có dấu**, tràn số là **hành vi không xác định (undefined behavior – UB)**. Compiler được phép giả định nó *không bao giờ xảy ra* và tối ưu dựa trên giả định đó, dẫn đến kết quả kỳ lạ.

```c
// overflow.c
#include <stdio.h>
#include <limits.h>

int main(void) {
    unsigned char u = 255;
    u = u + 1;                       // quay vòng, hợp lệ
    printf("unsigned char 255 + 1 = %d\n", u);   // 0

    int big = INT_MAX;               // 2147483647
    printf("INT_MAX = %d\n", big);
    // big + 1 là UB: KHÔNG làm điều này trong mã thật.
    // Cách kiểm tra an toàn: kiểm tra TRƯỚC khi cộng
    if (big > INT_MAX - 1) {
        printf("cong them 1 se bi tran\n");
    }
    return 0;
}
```

`<limits.h>` cung cấp các hằng như `INT_MIN`, `INT_MAX`, `CHAR_BIT` (số bit trong một byte, thường là 8).

## 2.3. Số thực (dấu phẩy động)

Số thực như 3,14 được lưu theo chuẩn **IEEE 754**, gồm ba phần: **dấu**, **số mũ** và **phần định trị (mantissa)**, tương tự ký hiệu khoa học `1.5 × 10³`, nhưng cơ số là 2.

| Kiểu | Số bit | Độ chính xác xấp xỉ |
|---|---|---|
| `float` | 32 | ~7 chữ số thập phân |
| `double` | 64 | ~15–16 chữ số thập phân |
| `long double` | 80 hoặc 128 (tùy nền tảng) | nhiều hơn |

Điều **cực kỳ quan trọng**: đa số số thập phân **không biểu diễn chính xác** trong hệ nhị phân, giống như 1/3 không viết chính xác được bằng thập phân hữu hạn.

```c
// float_trap.c
#include <stdio.h>
#include <math.h>

int main(void) {
    double a = 0.1 + 0.2;
    printf("0.1 + 0.2 = %.20f\n", a);       // 0.30000000000000004441
    printf("a == 0.3 ? %s\n", a == 0.3 ? "dung" : "sai");   // sai!

    // Cách so sánh đúng: so sánh với một sai số nhỏ (epsilon)
    if (fabs(a - 0.3) < 1e-9) {
        printf("gan bang 0.3 (trong sai so cho phep)\n");
    }
    return 0;
}
```

Biên dịch: `gcc -std=c11 -Wall -Wextra float_trap.c -o float_trap -lm` (`-lm` để liên kết thư viện toán `fabs`).

**Quy tắc:** *không bao giờ so sánh hai số thực bằng `==`.* Hãy so sánh giá trị tuyệt đối của hiệu với một ngưỡng sai số. Với tiền tệ, hãy lưu số nguyên (đơn vị nhỏ nhất, ví dụ đồng hoặc xu) thay vì `double`.

## 2.4. Ký tự và bảng mã

Máy tính chỉ hiểu số, nên ký tự được lưu dưới dạng **mã số** theo một bảng mã.

### ASCII

**ASCII** là bảng mã 7 bit gồm 128 ký tự (mã 0–127). Vài mã đáng nhớ:

| Ký tự | Mã (thập phân) | Ghi chú |
|---|---|---|
| `'0'` … `'9'` | 48 … 57 | Chữ số liên tiếp nhau |
| `'A'` … `'Z'` | 65 … 90 | |
| `'a'` … `'z'` | 97 … 122 | Chữ thường hơn chữ hoa đúng 32 |
| `' '` (dấu cách) | 32 | |
| `'\n'` | 10 | Xuống dòng |
| `'\0'` | 0 | Ký tự null |

Trong C, kiểu `char` thực chất là một **số nguyên nhỏ** (1 byte). `'A'` chính là số 65:

```c
// char_is_number.c
#include <stdio.h>

int main(void) {
    char c = 'A';
    printf("c = %c, ma = %d\n", c, c);      // c = A, ma = 65
    c = c + 1;
    printf("c + 1 = %c\n", c);              // B

    char digit = '7';
    int value = digit - '0';                // 7  (mẹo đổi ký tự số sang giá trị)
    printf("ky tu '%c' co gia tri so %d\n", digit, value);

    char lower = 'g';
    char upper = lower - ('a' - 'A');       // 'G'
    printf("%c -> %c\n", lower, upper);
    return 0;
}
```

Mẹo `digit - '0'` hoạt động vì các chữ số có mã liên tiếp.

### Unicode và UTF-8

ASCII không đủ cho tiếng Việt, tiếng Trung, emoji... **Unicode** gán mỗi ký tự một *code point* (ví dụ `ệ` là U+1EC7). **UTF-8** là cách mã hóa Unicode thành byte, độ dài **1 đến 4 byte** mỗi ký tự:

- Ký tự ASCII (`A`, `1`) chiếm 1 byte, giống hệt ASCII.
- Chữ có dấu tiếng Việt thường chiếm 2 hoặc 3 byte. `ệ` chiếm 3 byte.

Hệ quả cho người viết C: `strlen("Việt")` trả về số **byte**, không phải số ký tự; nó cho 6 chứ không phải 4. Chương 7 sẽ quay lại vấn đề này.

### Thư viện `<ctype.h>`

```c
#include <ctype.h>

isdigit(c)   // c là chữ số '0'..'9'?
isalpha(c)   // c là chữ cái (ASCII)?
isspace(c)   // c là khoảng trắng (dấu cách, tab, xuống dòng)?
toupper(c)   // đổi thành chữ hoa
tolower(c)   // đổi thành chữ thường
```

Các hàm này nhận `int`. Khi truyền `char` có thể âm, hãy ép sang `unsigned char`: `isdigit((unsigned char)c)`.

## 2.5. Kích thước kiểu dữ liệu và `sizeof`

Toán tử `sizeof` cho biết một kiểu hoặc biến chiếm **bao nhiêu byte**. Kết quả có kiểu `size_t` (số nguyên không dấu), in bằng `%zu`.

```c
// sizes.c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    printf("char        : %zu\n", sizeof(char));         // luôn là 1
    printf("short       : %zu\n", sizeof(short));
    printf("int         : %zu\n", sizeof(int));
    printf("long        : %zu\n", sizeof(long));
    printf("long long   : %zu\n", sizeof(long long));
    printf("float       : %zu\n", sizeof(float));
    printf("double      : %zu\n", sizeof(double));
    printf("void*       : %zu\n", sizeof(void *));       // kích thước một con trỏ
    printf("int32_t     : %zu\n", sizeof(int32_t));      // luôn là 4
    printf("int64_t     : %zu\n", sizeof(int64_t));      // luôn là 8

    int arr[10];
    printf("arr         : %zu byte, %zu phan tu\n", sizeof(arr), sizeof(arr) / sizeof(arr[0]));
    return 0;
}
```

Chuẩn C chỉ đảm bảo: `sizeof(char)` là 1 và `sizeof(short) ≤ sizeof(int) ≤ sizeof(long) ≤ sizeof(long long)`. Số byte cụ thể **tùy nền tảng**:

| Kiểu | Windows 64-bit | Linux/macOS 64-bit |
|---|---|---|
| `int` | 4 | 4 |
| `long` | **4** | **8** |
| `long long` | 8 | 8 |
| con trỏ | 8 | 8 |

Điểm khác biệt của `long` là bẫy kinh điển khi chuyển mã giữa Windows và Linux. Vì vậy, khi cần độ rộng cố định, hãy dùng các kiểu trong `<stdint.h>`: `int8_t`, `uint8_t`, `int16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`. Khi cần một số đo kích thước hoặc chỉ số mảng, dùng `size_t`.

Mẹo hay: `sizeof(arr) / sizeof(arr[0])` cho số phần tử của **mảng** (chỉ đúng khi `arr` là mảng thực sự, không phải con trỏ — xem chương 7 và 8).

## 2.6. Endianness — thứ tự byte trong bộ nhớ

Một `int` 32 bit gồm 4 byte. Vậy 4 byte đó nằm trong bộ nhớ theo thứ tự nào? Có hai cách:

- **Little-endian:** byte *ít quan trọng nhất* (LSB) lưu ở địa chỉ thấp nhất. Dùng bởi x86, x86-64, đa số ARM.
- **Big-endian:** byte *quan trọng nhất* (MSB) lưu ở địa chỉ thấp nhất. Dùng trong nhiều giao thức mạng (nên còn gọi là *network byte order*).

Ví dụ số `0x12345678`:

```text
Địa chỉ:          +0    +1    +2    +3
Little-endian:    78    56    34    12
Big-endian:       12    34    56    78
```

Kiểm chứng trên máy của bạn:

```c
// endian.c
#include <stdio.h>

int main(void) {
    unsigned int x = 0x12345678;
    unsigned char *p = (unsigned char *)&x;   // nhìn x như dãy byte

    for (size_t i = 0; i < sizeof(x); i++) {
        printf("byte %zu: 0x%02x\n", i, p[i]);
    }
    printf("May nay la %s-endian\n", p[0] == 0x78 ? "little" : "big");
    return 0;
}
```

Trên PC/laptop thông thường bạn sẽ thấy `78 56 34 12` — little-endian. Đây là lần đầu bạn dùng con trỏ; đừng lo nếu chưa hiểu hết, chương 8 sẽ giải thích. Điều cần nhớ: **endianness quan trọng khi ghi dữ liệu nhị phân ra file hoặc gửi qua mạng** giữa các máy khác nhau.

## 2.7. Bộ nhớ của một chương trình đang chạy

Khi chương trình chạy, hệ điều hành cấp cho nó một không gian bộ nhớ ảo, chia thành các vùng. Sơ đồ (địa chỉ thấp ở trên, cao ở dưới; chi tiết bố trí phụ thuộc hệ điều hành):

```text
địa chỉ thấp
┌──────────────────────────┐
│ Vùng mã (text)           │  các lệnh máy của chương trình (chỉ đọc)
├──────────────────────────┤
│ Dữ liệu khởi tạo (data)  │  biến toàn cục/static có giá trị ban đầu
├──────────────────────────┤
│ BSS                      │  biến toàn cục/static không khởi tạo (=0)
├──────────────────────────┤
│ Heap                     │  malloc/calloc/realloc — tăng dần ↓
│            ↓             │
│                          │
│            ↑             │
│ Stack                    │  biến cục bộ, tham số hàm — tăng dần ↑
└──────────────────────────┘
địa chỉ cao
```

### Stack (ngăn xếp)

- Lưu **biến cục bộ**, **tham số** và **địa chỉ trở về** của mỗi lần gọi hàm.
- Mỗi lần gọi hàm tạo ra một **stack frame**; khi hàm kết thúc, frame bị hủy. Do đó biến cục bộ **tự động** biến mất.
- Rất nhanh (chỉ dịch chuyển một thanh ghi), nhưng dung lượng **hạn chế** (thường 1–8 MB). Khai báo mảng cục bộ quá lớn hoặc đệ quy quá sâu gây **stack overflow**.

### Heap (vùng cấp phát động)

- Bộ nhớ bạn xin bằng `malloc`/`calloc`, và **phải tự trả** bằng `free`.
- Tồn tại cho đến khi bạn `free` (không mất khi hàm kết thúc), dung lượng lớn.
- Chậm hơn stack, và nếu quên `free` sẽ gây **rò rỉ bộ nhớ (memory leak)**.

### Minh họa

```c
// memory_regions.c
#include <stdio.h>
#include <stdlib.h>

int global_init = 42;        // vùng data
int global_uninit;           // vùng BSS (tự động = 0)

int main(void) {
    int local = 7;                                   // stack
    int *dyn = malloc(sizeof(int));                  // heap
    if (dyn == NULL) return 1;
    *dyn = 99;

    printf("ma chuong trinh (main)  : %p\n", (void *)main);
    printf("global_init  (data)     : %p\n", (void *)&global_init);
    printf("global_uninit (bss)     : %p\n", (void *)&global_uninit);
    printf("dyn -> (heap)           : %p\n", (void *)dyn);
    printf("local (stack)           : %p\n", (void *)&local);

    free(dyn);
    return 0;
}
```

Chạy nhiều lần và bạn sẽ thấy các địa chỉ nhóm lại theo vùng (và có thể thay đổi giữa các lần chạy do ASLR — cơ chế bảo mật xáo trộn địa chỉ).

Hai mô hình sử dụng:

```c
// stack: tự động
int add(int a, int b) {
    int result = a + b;   // result nằm trên stack, tự hủy khi hàm trả về
    return result;
}

// heap: thủ công
int *make_array(size_t n) {
    int *arr = malloc(n * sizeof(int));   // xin bộ nhớ trên heap
    return arr;                           // người gọi phải free(arr)
}
```

Chương 9 sẽ đi sâu vào quản lý bộ nhớ động.

## 2.8. Bức tranh về assembly và stack frame

Bạn không cần viết assembly, nhưng nhìn qua nó giúp hiểu C "dịch" thành gì. Tạo file:

```c
// add.c
int add(int a, int b) {
    return a + b;
}
```

Sinh assembly:

```bash
gcc -S -O0 -fno-asynchronous-unwind-tables add.c -o add.s
```

Trên x86-64 bạn sẽ thấy đại ý (rút gọn):

```asm
add:
    push rbp           ; lưu frame pointer cũ
    mov  rbp, rsp      ; tạo frame mới
    mov  DWORD PTR [rbp-4], edi    ; tham số a (truyền qua thanh ghi edi)
    mov  DWORD PTR [rbp-8], esi    ; tham số b (thanh ghi esi)
    mov  edx, DWORD PTR [rbp-4]
    mov  eax, DWORD PTR [rbp-8]
    add  eax, edx      ; a + b
    pop  rbp
    ret                ; quay về nơi gọi; giá trị trả về nằm trong eax
```

Ý chính: tham số truyền qua thanh ghi (theo quy ước gọi hàm của nền tảng), biến cục bộ nằm ở địa chỉ tương đối so với `rbp` trên stack, và giá trị trả về đặt trong thanh ghi `eax`. Đây là lý do trả về **địa chỉ của biến cục bộ** là lỗi: frame đã biến mất (xem chương 6 và 9).

## 2.9. An toàn khi làm việc với bộ nhớ

Ba thói quen phải hình thành ngay:

1. **Giới hạn khi đọc chuỗi:** dùng `fgets(buf, sizeof buf, stdin)`, không dùng `gets`.
2. **Kiểm tra `malloc`:** sau `malloc` phải kiểm tra `NULL` vì cấp phát có thể thất bại.
3. **Khởi tạo biến cục bộ:** biến cục bộ không tự động bằng 0; nó chứa "rác" cho đến khi bạn gán. Đọc biến chưa khởi tạo là lỗi.

```c
int x;               // giá trị không xác định
printf("%d\n", x);   // lỗi: đọc biến chưa khởi tạo (compiler có thể cảnh báo với -Wall)

int y = 0;           // đúng
```

## 2.10. Thực hành

### Thực hành 1: đổi số nguyên sang chuỗi nhị phân

```c
// bin_convert.c
#include <stdio.h>
#include <stdint.h>

// In n dưới dạng 32 bit nhị phân, nhóm mỗi 8 bit cho dễ đọc
void print_binary(uint32_t n) {
    for (int i = 31; i >= 0; i--) {
        putchar((n >> i) & 1 ? '1' : '0');   // dịch phải i bit, lấy bit thấp nhất
        if (i % 8 == 0 && i != 0) putchar(' ');
    }
    putchar('\n');
}

int main(void) {
    print_binary(13);          // 00000000 00000000 00000000 00001101
    print_binary(255);
    print_binary((uint32_t)-5);   // thấy biểu diễn bù hai của -5
    return 0;
}
```

Điểm học được: `n >> i` dịch bit sang phải `i` vị trí; `& 1` giữ lại bit thấp nhất. Bạn sẽ gặp lại các toán tử bit ở chương 4.

### Thực hành 2: bảng ASCII

```c
// ascii_table.c
#include <stdio.h>
#include <ctype.h>

int main(void) {
    printf("Ma  Ky tu\n");
    for (int c = 32; c < 127; c++) {       // 0..31 là ký tự điều khiển, không in được
        printf("%3d  %c\n", c, c);
    }
    return 0;
}
```

### Thực hành 3: `malloc`, `realloc`, `free`

```c
// demo_malloc.c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    size_t n = 5;
    int *arr = malloc(n * sizeof(int));
    if (arr == NULL) { perror("malloc"); return 1; }

    for (size_t i = 0; i < n; i++) arr[i] = (int)(i * i);

    // mở rộng lên 10 phần tử
    int *tmp = realloc(arr, 10 * sizeof(int));
    if (tmp == NULL) { free(arr); perror("realloc"); return 1; }
    arr = tmp;                                  // chỉ gán lại khi realloc thành công
    for (size_t i = n; i < 10; i++) arr[i] = (int)(i * i);

    for (size_t i = 0; i < 10; i++) printf("%d ", arr[i]);
    printf("\n");

    free(arr);
    arr = NULL;             // tránh con trỏ "treo" (dangling pointer)
    return 0;
}
```

Trên Linux kiểm tra rò rỉ: `gcc -g -o demo demo_malloc.c && valgrind --leak-check=full ./demo`. Kết quả mong đợi: `All heap blocks were freed -- no leaks are possible`.

## 2.11. Lỗi thường gặp

| Lỗi | Hậu quả | Cách tránh |
|---|---|---|
| So sánh `double` bằng `==` | Kết quả sai bất ngờ | So sánh với epsilon |
| Giả định `int` luôn 32 bit, `long` luôn 64 bit | Sai khi đổi nền tảng | Dùng `<stdint.h>` |
| Tràn số nguyên có dấu | UB, compiler tối ưu sai | Kiểm tra trước khi tính |
| `unsigned` trừ ra số âm | Kết quả thành số cực lớn | Cẩn thận khi trừ với `size_t` |
| Không kiểm tra `malloc` | Crash khi hết bộ nhớ | `if (p == NULL)` |
| Đọc biến chưa khởi tạo | Kết quả ngẫu nhiên | Luôn gán giá trị ban đầu |
| Quên `-lm` | `undefined reference to 'sqrt'` | Thêm `-lm` |

## 2.12. Tóm tắt

- Mọi dữ liệu là các bit; 8 bit = 1 byte; hex là cách viết gọn của nhị phân.
- Số âm dùng bù hai; tràn số có dấu là hành vi không xác định.
- Số thực là xấp xỉ nhị phân; không so sánh bằng `==`.
- `char` là số nguyên nhỏ; ký tự là mã số theo bảng mã (ASCII/UTF-8).
- `sizeof` cho kích thước theo byte; dùng `<stdint.h>` khi cần kích thước cố định.
- Stack chứa biến cục bộ (tự động); heap chứa dữ liệu cấp phát bằng `malloc` (phải `free`).

## 2.13. Bài tập

1. Đổi bằng tay các số sau sang nhị phân và hex: 10, 100, 255. Kiểm tra bằng chương trình `bases.c`.
2. Viết tay biểu diễn bù hai 8 bit của −1, −10, −128. Xác nhận bằng `print_binary`.
3. Viết chương trình đếm số bit `1` trong một số nguyên không dấu 32 bit.
4. Viết chương trình đọc một ký tự và in ra: là chữ hoa, chữ thường, chữ số hay ký tự khác (dùng `<ctype.h>`). Sau đó chuyển chữ hoa thành chữ thường **không dùng** `tolower` (dựa vào chênh lệch 32).
5. Chạy `sizes.c` trên máy của bạn và lập bảng. So sánh với bảng trong sách.
6. Sửa `endian.c` để in cả các byte của một `double` (ví dụ `1.0`). Tra cứu định dạng IEEE 754 để giải thích các byte thu được.
7. (Thử thách) Viết chương trình chứng minh `0.1f + 0.2f` khác `0.3f` và tìm giá trị `epsilon` nhỏ nhất làm phép so sánh gần đúng thành công.

Mã nguồn mẫu: /code/chapter-02
