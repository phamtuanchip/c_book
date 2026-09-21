# Chương 18 — Bảo mật & an toàn bộ nhớ

## Mục tiêu chương

- Hiểu vì sao C dễ có lỗ hổng an toàn bộ nhớ và các lớp lỗi chính: **tràn bộ đệm**, **format string**, **tràn số nguyên**, **use-after-free**, **race condition** (TOCTOU).
- Nắm cơ chế khai thác ở mức khái niệm (stack smashing) để hiểu **vì sao** các biện pháp phòng thủ tồn tại.
- Biết các lớp phòng thủ: **canary**, **NX/DEP**, **ASLR/PIE**, **RELRO**, **FORTIFY_SOURCE**, CFI.
- Viết mã C **an toàn theo mặc định**: kiểm tra biên, API an toàn, kiểm tra đầu vào, phép toán số học an toàn.
- Dùng công cụ: **cờ cảnh báo**, phân tích tĩnh, **ASan/UBSan/MSan**, **fuzzing** (libFuzzer/AFL++).
- Biết các nguyên tắc: đặc quyền tối thiểu, xóa bí mật khỏi bộ nhớ, tránh so sánh không hằng thời gian.

> **Lưu ý đạo đức:** nội dung chương nhằm giúp bạn **phòng thủ** và viết mã an toàn. Chỉ thử các ví dụ khai thác trên chương trình do chính bạn viết, trong môi trường thử nghiệm của bạn. Không dùng để tấn công hệ thống mà bạn không được phép.

## 18.1. Vì sao C dễ gặp lỗi bảo mật?

C cho phép truy cập bộ nhớ trực tiếp mà **không kiểm tra**: không giới hạn mảng, không kiểm tra con trỏ, không tự quản lý vòng đời bộ nhớ. Một lỗi nhỏ có thể biến thành **lỗ hổng**: kẻ tấn công cung cấp đầu vào được thiết kế khéo để đọc bộ nhớ ngoài ý muốn (rò rỉ thông tin), ghi đè dữ liệu/địa chỉ hàm (chiếm quyền điều khiển — *thực thi mã tùy ý*), hoặc làm chương trình sập (*từ chối dịch vụ*).

Các báo cáo của Microsoft và Google (Chromium) đều chỉ ra rằng khoảng **70%** lỗ hổng nghiêm trọng trong phần mềm viết bằng C/C++ là **lỗi an toàn bộ nhớ**. Đó là lý do có sự chuyển dịch sang ngôn ngữ như Rust — nhưng rất nhiều mã C sẽ còn sống hàng chục năm, nên kỹ năng viết C an toàn rất đáng giá.

### Mô hình đe dọa: đầu vào không tin cậy

Mọi dữ liệu từ **bên ngoài chương trình** đều **không tin cậy**: dòng lệnh, biến môi trường, file, socket, đường dẫn, dữ liệu từ thư viện khác, thậm chí tên file. Câu hỏi bảo mật cơ bản với mỗi dòng mã: *"Nếu kẻ tấn công kiểm soát giá trị này, điều tồi tệ nhất là gì?"*

## 18.2. Tràn bộ đệm (buffer overflow)

### Tràn trên stack — cơ chế

Ghi dữ liệu vượt quá kích thước mảng trên stack có thể ghi đè lên biến lân cận, **địa chỉ trở về** của hàm, hoặc các con trỏ.

```c
// vuln.c — CỐ Ý có lỗi để học. Không dùng mã này trong sản phẩm.
#include <stdio.h>
#include <string.h>

void greet(const char *name) {
    char buf[16];
    strcpy(buf, name);                   // KHÔNG kiểm tra độ dài
    printf("Xin chao, %s\n", buf);
}

int main(int argc, char *argv[]) {
    if (argc > 1) greet(argv[1]);
    return 0;
}
```

Bố cục stack của `greet` (địa chỉ cao ở trên):

```text
địa chỉ cao
┌──────────────────────────┐
│ tham số / frame của main │
├──────────────────────────┤
│ ĐỊA CHỈ TRỞ VỀ (→ main)  │  ◄─ kẻ tấn công muốn ghi đè cái này
├──────────────────────────┤
│ frame pointer đã lưu     │
├──────────────────────────┤
│ (canary — nếu bật)       │  ◄─ giá trị bí mật, kiểm tra trước khi return
├──────────────────────────┤
│ buf[16]                  │  ◄─ strcpy ghi từ đây, đi LÊN phía địa chỉ cao
└──────────────────────────┘
địa chỉ thấp
```

Nếu `name` dài hơn 15 ký tự, `strcpy` ghi tràn `buf`, lần lượt phá canary, frame pointer, rồi **địa chỉ trở về**. Khi `greet` kết thúc và thực hiện `ret`, CPU nhảy tới địa chỉ mà kẻ tấn công đã chọn. Đó là *stack smashing*. Ngay cả khi không bị khai thác, chương trình thường bị crash (`Segmentation fault`).

Xem hiệu ứng an toàn (không khai thác): biên dịch với ASan:

```bash
gcc -std=c11 -g -fsanitize=address vuln.c -o vuln
./vuln AAAAAAAAAAAAAAAAAAAAAAAAAAAA
# ERROR: AddressSanitizer: stack-buffer-overflow ... WRITE of size 29 ... in strcpy
```

### Tràn trên heap

```c
char *name = malloc(8);
strcpy(name, "nguyen van a");           // 13 byte vào khối 8 byte -> phá metadata của allocator
```

Tràn heap có thể ghi đè dữ liệu của đối tượng khác cấp phát ngay sau đó (ví dụ một con trỏ hàm, cờ "is_admin") hoặc cấu trúc quản lý của `malloc`. Khó khai thác hơn nhưng vẫn nguy hiểm.

### Các hàm nguy hiểm và thay thế an toàn

| Nguy hiểm | Lý do | Thay bằng |
|---|---|---|
| `gets(buf)` | Không giới hạn; **đã bị xóa khỏi C11** | `fgets(buf, sizeof buf, stdin)` |
| `strcpy(dst, src)` | Không kiểm tra độ dài | `snprintf(dst, sizeof dst, "%s", src)`; `memcpy` sau khi kiểm tra; `strlcpy` nếu có |
| `strcat(dst, src)` | Như trên | `snprintf` với offset; `strlcat` |
| `sprintf(buf, fmt, ...)` | Không giới hạn | `snprintf(buf, sizeof buf, fmt, ...)` |
| `scanf("%s", buf)` | Không giới hạn | `scanf("%15s", buf)` (số = kích thước − 1) hoặc `fgets` |
| `strncpy(dst, src, n)` | Có thể **không thêm `'\0'`**; đệm 0 thừa | `snprintf`; hoặc luôn gán `dst[n-1] = '\0'` |
| `atoi`, `atof` | Không báo lỗi | `strtol`, `strtod` (kiểm tra `errno`, `end`) |
| `system(cmd)` | Command injection | Hàm `exec*`/`posix_spawn` với mảng đối số |
| `mktemp` | Race/đoán được | `mkstemp` |
| `tmpnam` | Race/đoán được | `mkstemp` / `tmpfile` |
| `rand()` cho bí mật | Đoán được | `getrandom`, `/dev/urandom`, `arc4random` |

### Nguyên tắc chống tràn

1. **Luôn biết kích thước đích** và truyền nó cùng con trỏ: `(char *buf, size_t cap)`.
2. **Kiểm tra trước khi chép:** `if (len >= cap) return ERROR;` (nhớ `+1` cho `'\0'`).
3. **Dùng độ dài tường minh** thay vì dựa vào `'\0'` khi dữ liệu không tin cậy (có thể không có `'\0'`): `strnlen`, `memchr`.
4. **Mảng có kích thước cố định + `sizeof`**, không "số ma thuật" lặp lại nhiều nơi.
5. **Đóng gói** buffer cùng độ dài vào struct rồi chỉ truy cập qua hàm có kiểm tra biên:

```c
typedef struct { char *data; size_t len, cap; } Buf;

int buf_append(Buf *b, const void *src, size_t n) {
    if (n > b->cap - b->len) return -1;          // so sánh không tràn: cap - len không âm vì len <= cap
    memcpy(b->data + b->len, src, n);
    b->len += n;
    return 0;
}
```

Chú ý cách viết `n > cap - len` thay vì `len + n > cap` (biểu thức sau có thể **tràn số** khi `n` rất lớn, xem 18.4).

### Ví dụ vá lỗi `vuln.c`

```c
void greet_safe(const char *name) {
    char buf[16];
    int n = snprintf(buf, sizeof buf, "%s", name);   // luôn kết thúc bằng '\0', không ghi quá 16 byte
    if (n < 0 || (size_t)n >= sizeof buf) {
        fprintf(stderr, "ten qua dai, da cat bot\n");
    }
    printf("Xin chao, %s\n", buf);
}
```

Hoặc từ chối hẳn đầu vào quá dài nếu việc cắt bớt gây sai nghiệp vụ.

## 18.3. Lỗ hổng chuỗi định dạng (format string)

```c
// SAI
char input[256];
fgets(input, sizeof input, stdin);
printf(input);                        // input do người dùng quyết định định dạng!
```

Nếu `input` chứa `%x %x %x %x`, `printf` sẽ **đọc các giá trị trên stack** và in ra — **rò rỉ thông tin** (địa chỉ, canary, dữ liệu). Với `%s` lấy con trỏ từ stack có thể crash; với **`%n`** (ghi số ký tự đã in vào địa chỉ trên stack) kẻ tấn công có thể **ghi vào bộ nhớ tùy ý**.

**Cách đúng:** luôn dùng chuỗi định dạng **hằng số** và truyền dữ liệu như đối số:

```c
printf("%s", input);                  // ĐÚNG
puts(input);                          // hoặc fputs(input, stdout)
```

Quy tắc: **chuỗi định dạng không bao giờ được đến từ đầu vào không tin cậy.** Áp dụng cho cả `fprintf`, `sprintf`, `snprintf`, `syslog`, và các hàm log tự viết. Bật `-Wformat -Wformat-security -Werror=format-security`: gcc sẽ cảnh báo `printf(input)`. Với hàm log tự viết, gắn `__attribute__((format(printf, ...)))` (chương 13) để compiler kiểm tra.

## 18.4. Tràn số nguyên (integer overflow)

Tràn số nguyên không tự nó phá bộ nhớ, nhưng khi kết quả dùng để **tính kích thước cấp phát hoặc chỉ số**, nó dẫn tới tràn bộ đệm.

### Ví dụ: cấp phát bị tràn

```c
// SAI
uint32_t count = read_count_from_network();       // kẻ tấn công chọn: 0x40000001
uint32_t size  = count * 4;                       // 0x100000004 cắt về 32 bit thành 4!
uint8_t *buf   = malloc(size);                    // chỉ 4 byte
for (uint32_t i = 0; i < count; i++)
    memcpy(buf + i * 4, src + i * 4, 4);          // ghi cả tỷ lần ra ngoài 4 byte -> tràn heap
```

Phép nhân tràn nhưng vẫn "thành công", nên `malloc` cấp ít hơn nhiều so với số vòng lặp.

### Cách tránh

**1. Kiểm tra trước khi tính:**

```c
#include <stdint.h>
#include <stdlib.h>

int alloc_array(size_t count, size_t elem, void **out) {
    if (elem != 0 && count > SIZE_MAX / elem) return -1;   // count * elem sẽ tràn
    void *p = malloc(count * elem);
    if (!p) return -1;
    *out = p;
    return 0;
}
```

**2. Dùng hàm nhận nhân sẵn có kiểm tra:** `calloc(count, size)` tự kiểm tra tràn của phép nhân.

**3. Dùng builtin của compiler (gcc/clang):**

```c
size_t total;
if (__builtin_mul_overflow(count, elem, &total)) return -1;
if (__builtin_add_overflow(a, b, &sum))          return -1;
```

C23 chuẩn hóa `<stdckdint.h>` (`ckd_add`, `ckd_mul`).

**4. Với số có dấu:** kiểm tra **trước** khi thực hiện phép toán (vì tràn số có dấu là UB, không thể kiểm tra sau):

```c
int safe_add(int a, int b, int *out) {
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) return -1;
    *out = a + b;
    return 0;
}
```

### Lỗi dấu (sign) và ép kiểu

```c
int len = get_length();                  // có thể âm!
char buf[64];
if (len > 64) return;                    // -1 vượt qua kiểm tra
memcpy(buf, src, len);                   // len chuyển thành size_t: -1 -> 18446744073709551615
```

Sửa: kiểm tra **cả hai phía** (`len < 0 || len > 64`) hoặc dùng kiểu không dấu ngay từ đầu (`size_t`) và không trộn.

Truncation cũng nguy hiểm: `unsigned char n = len;` cắt `len = 300` thành `44`. Dùng `-Wconversion -Wsign-conversion` để compiler bắt các chuyển đổi ngầm.

## 18.5. Use-after-free, double free và các lỗi vòng đời (nhắc lại)

Chương 9 đã trình bày các lỗi này. Ở góc độ bảo mật:

- **Use-after-free:** kẻ tấn công **đặt dữ liệu của mình vào vùng nhớ vừa giải phóng** (bằng cách gây ra một cấp phát mới cùng kích thước), rồi con trỏ treo của bạn dùng dữ liệu đó (ví dụ gọi qua **con trỏ hàm** nằm trong đối tượng). Đây là một trong những lớp lỗ hổng bị khai thác nhiều nhất trong trình duyệt và nhân.
- **Double free:** phá cấu trúc danh sách trống của allocator, dẫn tới ghi tùy ý.
- **Đọc chưa khởi tạo (uninitialized read):** rò rỉ dữ liệu cũ trên stack/heap, có thể chứa khóa/mật khẩu; và struct có **padding** khi gửi ra ngoài (ghi file/socket) làm lộ byte rác — hãy `memset(&s, 0, sizeof s)` hoặc khởi tạo `= {0}` trước khi điền.
- **Out-of-bounds read** (như *Heartbleed*): đọc quá độ dài dữ liệu thật rồi gửi trả — rò rỉ nội dung bộ nhớ. Nguyên nhân: tin độ dài do đối phương tự khai (xem `recv_msg` chương 16: luôn **kiểm tra độ dài khai báo** với dữ liệu thực có).

Biện pháp mã: gán `NULL` sau `free`, ownership rõ ràng, tránh con trỏ thô chia sẻ, dùng ASan trong kiểm thử.

## 18.6. Race condition bảo mật: TOCTOU

**Time-Of-Check to Time-Of-Use:** kiểm tra một điều kiện rồi mới dùng, nhưng giữa hai bước trạng thái đã đổi.

```c
// SAI: chương trình chạy với đặc quyền cao
if (access(path, W_OK) == 0) {           // 1) kiểm tra: người dùng có quyền ghi file này không?
    FILE *f = fopen(path, "w");          // 2) dùng: nhưng giữa 1 và 2, kẻ tấn công thay path bằng
    ...                                  //    symlink trỏ tới /etc/passwd -> ghi vào file hệ thống!
}
```

Cách đúng: **bỏ bước kiểm tra riêng**; mở file **trực tiếp** với cờ phù hợp rồi xử lý lỗi, và dùng cờ chống symlink/đảm bảo tính duy nhất:

```c
int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
if (fd < 0) { perror("open"); return -1; }
```

Với file tạm: dùng `mkstemp`, không dùng `tmpnam`/`mktemp`. Hạ đặc quyền (`setuid`) trước khi mở file thay mặt người dùng.

## 18.7. Command injection, path traversal, và tin cậy đầu vào

### Command injection

```c
// SAI
char cmd[256];
snprintf(cmd, sizeof cmd, "ls -l %s", user_supplied_dir);
system(cmd);                             // nếu dir là "x; rm -rf ~" -> chạy cả lệnh thứ hai!
```

Tránh `system()`/`popen()` với dữ liệu ngoài. Dùng `fork` + `execv` (truyền **mảng đối số**, không qua shell) hoặc `posix_spawn`; tốt hơn cả là dùng API trực tiếp (`opendir`) thay vì chạy công cụ ngoài.

### Path traversal

```c
// SAI: người dùng gửi tên file "../../etc/passwd"
snprintf(path, sizeof path, "/var/www/files/%s", name);
```

Cách phòng: từ chối tên chứa `/`, `\`, `..`, hoặc chuẩn hóa bằng `realpath()` rồi kiểm tra kết quả nằm trong thư mục gốc cho phép (`strncmp(resolved, root, strlen(root)) == 0` và ký tự kế tiếp là `/` hoặc `'\0'`).

### Validate theo danh sách cho phép (allow-list)

Kiểm tra đầu vào bằng cách liệt kê **những gì được phép**, không cố liệt kê những gì cấm:

```c
int is_valid_username(const char *s) {
    size_t n = strlen(s);
    if (n == 0 || n > 32) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}
```

Quy tắc: **validate ở biên (khi dữ liệu vào)**, **chuẩn hóa trước khi kiểm tra**, và **kiểm tra độ dài, kiểu, phạm vi, định dạng**.

## 18.8. Các lớp phòng thủ của hệ thống và compiler

Không thay thế cho việc viết mã đúng, nhưng **làm khai thác khó hơn nhiều**. Nên bật tất cả trong bản phát hành.

| Cơ chế | Chống gì | Cách bật (gcc/clang) |
|---|---|---|
| **Stack canary** | Ghi đè địa chỉ trở về: chèn giá trị bí mật trước địa chỉ trở về, kiểm tra trước khi `ret`; sai → dừng | `-fstack-protector-strong` |
| **NX / DEP** (Non-eXecutable) | Chạy mã kẻ tấn công chèn vào stack/heap: đánh dấu vùng dữ liệu không thực thi | Mặc định (linker: `-z noexecstack`) |
| **ASLR** (Address Space Layout Randomization) | Kẻ tấn công không biết địa chỉ cố định: xáo trộn địa chỉ stack, heap, thư viện mỗi lần chạy | Hệ điều hành (`/proc/sys/kernel/randomize_va_space`) |
| **PIE** (Position Independent Executable) | Để chính chương trình cũng được ASLR | `-fPIE -pie` |
| **RELRO** (Relocation Read-Only) | Ghi đè bảng GOT: đánh dấu chỉ đọc sau khi nạp | `-Wl,-z,relro,-z,now` (Full RELRO) |
| **FORTIFY_SOURCE** | Kiểm tra biên tự động cho `memcpy`, `strcpy`, `sprintf`... khi biết kích thước đích lúc biên dịch | `-O2 -D_FORTIFY_SOURCE=2` (hoặc `=3`) |
| **CFI** (Control-Flow Integrity) | Nhảy tới địa chỉ hàm không hợp lệ (ROP/JOP) | `-fsanitize=cfi -flto` (clang) |
| **Shadow stack / CET** | Sửa địa chỉ trở về | Phần cứng + hệ điều hành mới |
| **`-ftrivial-auto-var-init=zero`** | Đọc biến cục bộ chưa khởi tạo | gcc 12+/clang: khởi tạo biến tự động về 0 |

Bộ cờ khuyến nghị cho bản phát hành gcc/clang trên Linux:

```bash
gcc -std=c11 -O2 -Wall -Wextra -Wformat=2 -Wformat-security \
    -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE -pie \
    -Wl,-z,relro,-z,now,-z,noexecstack \
    prog.c -o prog
```

Kiểm tra một file nhị phân đã bật những gì: `checksec --file=./prog` (công cụ `checksec`), hoặc `readelf -l`, `readelf -d`.

**Lưu ý:** các biện pháp này là **giảm thiểu (mitigation)**, không phải sửa lỗi. Kẻ tấn công có thể vượt qua chúng khi có thêm lỗ hổng rò rỉ địa chỉ. Mã đúng vẫn là tuyến phòng thủ đầu tiên.

## 18.9. Công cụ phát hiện lỗi

### Cảnh báo compiler — bật hết và coi trọng

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion \
    -Wformat=2 -Wcast-qual -Wcast-align -Wstrict-prototypes -Wnull-dereference \
    -Wdouble-promotion -Wundef -Wvla -Werror prog.c
```

(Mỗi cờ thêm nhiều cảnh báo; cấp độ `-Wconversion` khá ồn nên áp dụng dần.) `gcc -fanalyzer` (10+) chạy phân tích tĩnh liên thủ tục, phát hiện double free, rò rỉ, NULL dereference, use-after-free.

### Phân tích tĩnh

```bash
cppcheck --enable=warning,style,performance,portability --std=c11 src/
clang-tidy -checks='clang-analyzer-*,bugprone-*,cert-*' src/*.c -- -std=c11
clang --analyze src/*.c            # Clang Static Analyzer
```

Có nhiều kết quả **dương tính giả (false positive)**, và cũng có lỗi mà chúng **không thấy** — vẫn cần kiểm thử động.

### Sanitizer (phân tích động, khi chạy)

| Sanitizer | Cờ | Bắt được |
|---|---|---|
| **AddressSanitizer (ASan)** | `-fsanitize=address` | Tràn stack/heap/global, use-after-free, double free, rò rỉ (LeakSanitizer) |
| **UndefinedBehaviorSanitizer (UBSan)** | `-fsanitize=undefined` | Tràn số có dấu, dịch bit sai, NULL dereference, chia cho 0, ép kiểu sai |
| **MemorySanitizer (MSan)** (clang) | `-fsanitize=memory` | Đọc bộ nhớ chưa khởi tạo |
| **ThreadSanitizer (TSan)** | `-fsanitize=thread` | Data race (chương 15) |
| **Hardware-assisted ASan / MTE** | ARM | Nhẹ hơn, dùng cả trong sản phẩm |

```bash
gcc -std=c11 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer prog.c -o prog
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 ./prog
```

Cách đọc báo cáo ASan (chương 9): loại lỗi, dòng gây lỗi, dòng cấp phát/giải phóng liên quan.

### Fuzzing — tìm lỗi bằng đầu vào ngẫu nhiên có hướng dẫn

**Fuzzer** sinh hàng triệu đầu vào biến đổi, theo dõi **độ phủ mã** để ưu tiên đầu vào mở ra nhánh mới, và báo khi chương trình crash hoặc sanitizer kêu. Đây là công cụ rất hiệu quả cho **bộ phân tích dữ liệu** (parser, giải mã, giao thức).

**libFuzzer** (clang) — bạn viết một hàm nhận mảng byte:

```c
/* fuzz_parse.c (mẫu cho libFuzzer; cần parser.h của bạn) */
#include <stddef.h>
#include <stdint.h>
#include "parser.h"          // hàm cần thử: parse_config(const char *data, size_t len)

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    parse_config((const char *)data, size);     // không được crash, treo, hay rò rỉ với BẤT KỲ đầu vào nào
    return 0;
}
```

```bash
clang -std=c11 -g -O1 -fsanitize=fuzzer,address,undefined fuzz_parse.c parser.c -o fuzz_parse
mkdir corpus && ./fuzz_parse corpus/ -max_total_time=60
# crash-xxxx: file chứa đầu vào gây lỗi để tái hiện: ./fuzz_parse crash-xxxx
```

**AFL++** thay thế: `afl-clang-fast` để biên dịch, `afl-fuzz -i in -o out -- ./prog @@`. Đặt thư mục `in/` với vài file mẫu hợp lệ làm hạt giống.

Fuzzing nên tích hợp vào CI (chương 21) và chạy định kỳ; giữ lại các đầu vào từng gây crash làm **bộ kiểm thử hồi quy**.

### Valgrind

`valgrind --leak-check=full --track-origins=yes ./prog` — không cần biên dịch lại, chậm hơn nhưng bắt cả đọc chưa khởi tạo.

## 18.10. Thực hành lập trình an toàn

### Danh sách kiểm tra

1. **Coi mọi đầu vào ngoài là không tin cậy;** kiểm tra độ dài, phạm vi, định dạng ngay tại biên.
2. **Luôn có giới hạn:** kích thước bộ đệm, độ dài dòng/thông điệp, số kết nối, độ sâu đệ quy, thời gian.
3. **Truyền kích thước cùng con trỏ;** không dựa vào `'\0'` cho dữ liệu không tin cậy.
4. **Khởi tạo mọi thứ:** biến, struct (`= {0}`), bộ nhớ (`calloc`), và `memset` struct gửi ra ngoài.
5. **Kiểm tra mọi giá trị trả về** (nhất là `malloc`, `read`, `fopen`, `snprintf`).
6. **Kiểm tra tràn số trước khi dùng cho kích thước/chỉ số;** dùng `size_t` nhất quán, tránh trộn signed/unsigned.
7. **Cấp phát và giải phóng đối xứng;** ownership rõ; `NULL` sau `free`.
8. **Chuỗi định dạng là hằng số.**
9. **Ưu tiên API an toàn** (`snprintf`, `fgets`, `strtol`, `mkstemp`, `calloc`), tránh danh sách ở 18.2.
10. **Đặc quyền tối thiểu:** không chạy với quyền cao hơn cần thiết; hạ quyền sớm (`setuid`), bỏ khả năng không dùng (`seccomp`, `pledge`/`unveil` trên OpenBSD).
11. **Đơn giản hóa:** mã phức tạp che giấu lỗi. Chia nhỏ hàm, ít con trỏ nhiều tầng.
12. **Bật mọi cờ cảnh báo và sanitizer trong CI;** chạy fuzzer với parser.
13. **Xem xét mã (code review)** tập trung vào ranh giới tin cậy.
14. **Cập nhật phụ thuộc** và theo dõi CVE của thư viện bạn dùng.

### Chuẩn viết mã an toàn

- **CERT C Coding Standard** (SEI): danh sách quy tắc và khuyến nghị cụ thể, kèm ví dụ mã sai/đúng.
- **MISRA C**: dùng nhiều trong ô tô, hàng không, y tế.
- **CWE Top 25** và **OWASP**: danh mục lỗ hổng phổ biến để đối chiếu.

## 18.11. Xử lý dữ liệu nhạy cảm

### Xóa bí mật khỏi bộ nhớ

Mật khẩu, khóa, token không nên nằm lại trong RAM lâu hơn cần thiết (có thể lộ qua core dump, swap, lỗ hổng đọc bộ nhớ). Nhưng:

```c
char password[64];
/* ... dùng xong ... */
memset(password, 0, sizeof password);      // compiler có thể XÓA lệnh này (dead store elimination) vì password không được đọc nữa!
```

Dùng hàm **được đảm bảo không bị tối ưu bỏ**: `explicit_bzero` (glibc, BSD), `memset_s` (C11 Annex K), `SecureZeroMemory` (Windows), hoặc tự viết qua con trỏ `volatile`:

```c
static void secure_zero(void *p, size_t n) {
    volatile unsigned char *v = p;
    while (n--) *v++ = 0;
}
```

Đồng thời khóa vùng nhớ nhạy cảm khỏi swap: `mlock`.

### So sánh không hằng thời gian và tấn công kênh phụ

`memcmp`/`strcmp` dừng ngay khi gặp byte khác nhau, nên **thời gian chạy tiết lộ** bao nhiêu byte đầu đúng — kẻ tấn công có thể đoán từng byte của token/MAC (*timing attack*). Khi so sánh bí mật, dùng phép so sánh **hằng thời gian**:

```c
int consteq(const void *a, const void *b, size_t n) {
    const unsigned char *x = a, *y = b;
    unsigned char diff = 0;
    for (size_t i = 0; i < n; i++) diff |= x[i] ^ y[i];   // luôn duyệt hết n byte
    return diff == 0;
}
```

(Hoặc `CRYPTO_memcmp` của OpenSSL, `sodium_memcmp` của libsodium.)

### Ngẫu nhiên và mật mã

- **Không dùng `rand()`/`srand(time(NULL))` cho bí mật** — dễ đoán. Dùng `getrandom()`, `/dev/urandom`, `arc4random_buf`, `BCryptGenRandom` (Windows).
- **Không tự phát minh mật mã.** Dùng thư viện đã được kiểm chứng (libsodium, OpenSSL, mbedTLS) và giao thức tiêu chuẩn (TLS). Băm mật khẩu bằng Argon2/bcrypt/scrypt, không dùng MD5/SHA-1/SHA-256 đơn thuần.

## 18.12. Ví dụ hoàn chỉnh: đọc thông điệp có tiền tố độ dài an toàn

Đây là mẫu của lỗi Heartbleed: gói tin khai độ dài `payload_len` nhưng dữ liệu thực có thể ngắn hơn. Đây là phiên bản **an toàn**:

```c
// safe_parse.c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PAYLOAD 4096

typedef struct {
    uint8_t  type;
    uint16_t len;                       // độ dài payload do đối phương khai
    uint8_t  payload[MAX_PAYLOAD];
} Message;

typedef enum { P_OK = 0, P_SHORT, P_TOO_BIG, P_BAD_TYPE } ParseResult;

// data/size: gói tin nhận được (KHÔNG tin cậy). Trả kết quả phân tích vào *out.
ParseResult parse_message(const uint8_t *data, size_t size, Message *out) {
    memset(out, 0, sizeof *out);                   // khởi tạo sạch, không để lộ byte cũ

    if (size < 3) return P_SHORT;                  // cần ít nhất header 3 byte: type(1) + len(2)

    uint8_t  type = data[0];
    uint16_t len  = (uint16_t)((data[1] << 8) | data[2]);     // big-endian, đọc từng byte (không ép kiểu con trỏ)

    if (type > 3)            return P_BAD_TYPE;    // allow-list các giá trị hợp lệ
    if (len > MAX_PAYLOAD)   return P_TOO_BIG;     // giới hạn cứng
    if ((size_t)len > size - 3) return P_SHORT;    // độ dài khai báo KHÔNG được vượt quá dữ liệu thực có
                                                   // (viết size - 3 thay vì 3 + len để tránh tràn)
    out->type = type;
    out->len  = len;
    memcpy(out->payload, data + 3, len);
    return P_OK;
}

int main(void) {
    uint8_t good[] = { 1, 0x00, 0x05, 'h', 'e', 'l', 'l', 'o' };
    uint8_t liar[] = { 1, 0xFF, 0xFF, 'x' };       // khai 65535 byte nhưng chỉ có 1
    Message m;
    printf("good: %d\n", parse_message(good, sizeof good, &m));   // 0
    printf("liar: %d\n", parse_message(liar, sizeof liar, &m));   // 2 (P_TOO_BIG) chặn ngay
    return 0;
}
```

Các điểm an toàn: kiểm tra kích thước tối thiểu trước khi đọc header, đọc số nhiều byte từng byte (tránh vấn đề căn hàng và endianness), giới hạn độ dài, so sánh độ dài khai báo với dữ liệu thực bằng biểu thức không tràn, `memset` đầu ra. Đây cũng là mục tiêu lý tưởng để **fuzz** (mục 18.9).

## 18.13. Lỗi thường gặp

| Lỗi | Hậu quả | Cách tránh |
|---|---|---|
| `gets`, `strcpy`, `sprintf`, `scanf("%s")` | Tràn bộ đệm | `fgets`, `snprintf`, giới hạn độ rộng |
| `printf(user_input)` | Rò rỉ/ghi bộ nhớ | `printf("%s", user_input)` |
| `count * size` không kiểm tra tràn | Cấp phát nhỏ, ghi tràn | `calloc`, `__builtin_mul_overflow` |
| `len < max` với `len` có dấu | `-1` vượt qua, thành số khổng lồ | Kiểm tra hai phía hoặc dùng `size_t` |
| Tin độ dài do đối phương khai | Đọc/ghi ngoài vùng (Heartbleed) | So với dữ liệu thực có, giới hạn tối đa |
| `system()` với dữ liệu ngoài | Command injection | `execv`, API trực tiếp |
| Ghép đường dẫn với tên do người dùng đưa | Path traversal | `realpath` + kiểm tra thư mục gốc |
| `access()` rồi `open()` | TOCTOU | Mở trực tiếp với `O_EXCL/O_NOFOLLOW` |
| `memset` bí mật rồi không dùng nữa | Bị compiler xóa | `explicit_bzero`/`memset_s` |
| Dùng `rand()` cho token | Đoán được | `getrandom`/`/dev/urandom` |
| So sánh bí mật bằng `memcmp` | Timing attack | So sánh hằng thời gian |
| Chỉ dựa vào canary/ASLR | Vẫn bị vượt qua | Sửa lỗi gốc; nhiều lớp phòng thủ |

## 18.14. Tóm tắt

- Lỗ hổng C chủ yếu là **lỗi bộ nhớ**: tràn bộ đệm, format string, tràn số nguyên → tràn, use-after-free, đọc chưa khởi tạo, TOCTOU.
- **Mọi dữ liệu ngoài là không tin cậy;** validate theo allow-list, giới hạn kích thước ở mọi nơi, kiểm tra tràn số **trước** khi tính.
- Tránh hàm không an toàn; dùng `snprintf`, `fgets`, `strtol`, `calloc`, `mkstemp`; chuỗi định dạng phải là hằng.
- Bật các lớp phòng thủ: canary, NX, ASLR/PIE, RELRO, FORTIFY_SOURCE — nhưng chúng chỉ **giảm thiểu**.
- Phát hiện lỗi: cảnh báo đầy đủ, phân tích tĩnh, **ASan/UBSan/MSan/TSan**, **fuzzing**; tích hợp vào CI.
- Xử lý bí mật: xóa an toàn, so sánh hằng thời gian, ngẫu nhiên mật mã, dùng thư viện mật mã có sẵn.

## 18.15. Bài tập

1. Biên dịch `vuln.c` với ASan, gọi với chuỗi dài, đọc báo cáo và mô tả từng phần. Sửa bằng `snprintf` và xác nhận báo cáo sạch. Sau đó biên dịch **không** có `-fstack-protector`, rồi có, và so sánh hành vi khi tràn (dùng `-fno-stack-protector` / `-fstack-protector-strong`).
2. Viết chương trình có lỗi format string (`printf(buf)`), dùng `%x %x %x` để quan sát rò rỉ (chỉ trên máy bạn), rồi vá và bật `-Werror=format-security` để compiler bắt lỗi.
3. Viết hàm `void *xmalloc_array(size_t n, size_t size)` an toàn tràn số, và bộ kiểm thử chứng minh nó trả `NULL` với `n = SIZE_MAX/2 + 1, size = 2`. Dùng thêm `__builtin_mul_overflow` để so sánh.
4. Viết `safe_add`, `safe_mul` cho `int` (không UB) và kiểm thử ở các biên `INT_MAX`, `INT_MIN`. Kiểm tra bằng UBSan.
5. Viết chương trình "tải file cho người dùng" nhận tên file, chống path traversal bằng `realpath` và kiểm tra tiền tố; thử với `../../etc/passwd` và symlink.
6. Viết fuzz target cho `csv_split` (chương 12) hoặc `parse_message`; chạy libFuzzer 5 phút với ASan+UBSan. Nếu tìm được lỗi, phân tích và sửa; lưu đầu vào gây lỗi làm test hồi quy.
7. Viết `consteq` và so sánh thời gian với `memcmp` khi hai chuỗi khác nhau ở byte đầu và byte cuối (đo nhiều lần).
8. Viết hàm đọc mật khẩu từ bàn phím (tắt echo bằng `termios`), lưu vào bộ đệm, dùng xong xóa bằng `explicit_bzero`/`secure_zero`; kiểm tra bằng `gcc -O2 -S` để thấy `memset` thường bị bỏ còn `secure_zero` thì không.
9. (Thử thách) Kiểm tra một dự án C mã nguồn mở nhỏ bằng `cppcheck`, `clang-tidy`, `-fanalyzer`, ASan+UBSan và fuzz; lập báo cáo phân loại các phát hiện (thật/dương tính giả) và đề xuất bản vá.

Mã nguồn mẫu: /code/chapter-18
