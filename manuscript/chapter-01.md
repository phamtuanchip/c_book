# Chương 1 — Giới thiệu và chương trình C đầu tiên

## Mục tiêu chương

Sau khi học xong chương này, bạn sẽ:

- Hiểu C là gì, được dùng ở đâu và vì sao vẫn đáng học ở thời điểm hiện tại.
- Cài đặt môi trường phát triển trên Windows, Linux hoặc macOS và kiểm tra nó hoạt động.
- Viết, biên dịch, chạy chương trình C đầu tiên và hiểu **từng dòng** của nó.
- Hiểu quá trình từ file `.c` đến file chạy được gồm những giai đoạn nào.
- Đọc được thông báo lỗi và cảnh báo của compiler thay vì sợ chúng.

> **Cách học chương này:** hãy gõ lại mọi ví dụ bằng tay, đừng copy-paste. Việc tự gõ giúp bạn nhớ cú pháp và làm quen với việc compiler báo lỗi khi bạn gõ sai.

## 1.1. C là gì và vì sao học C?

**C** là ngôn ngữ lập trình được Dennis Ritchie tạo ra tại Bell Labs vào khoảng năm 1972 để viết lại hệ điều hành UNIX. Đến nay, hơn 50 năm sau, C vẫn nằm trong nhóm ngôn ngữ được dùng nhiều nhất.

C thuộc loại **ngôn ngữ biên dịch (compiled language)**: mã nguồn bạn viết được một chương trình gọi là *compiler* dịch thành **mã máy** (các lệnh mà CPU hiểu trực tiếp). Kết quả là chương trình chạy nhanh và không cần "máy ảo" hay "trình thông dịch" đi kèm.

### Nơi C được sử dụng

| Lĩnh vực | Ví dụ |
|---|---|
| Hệ điều hành | Nhân Linux, phần lớn nhân Windows, macOS/iOS, Android (phần lõi) |
| Hệ thống nhúng | Vi điều khiển trong máy giặt, ô tô, thiết bị y tế, Arduino |
| Cơ sở dữ liệu | SQLite, PostgreSQL, Redis, MySQL |
| Trình biên dịch và runtime | CPython (bản Python chuẩn), Ruby, PHP, Lua là các trình thông dịch viết bằng C |
| Mạng | nginx, OpenSSL, curl |
| Game engine, driver | Phần lõi hiệu năng cao |

### Học C mang lại điều gì?

1. **Hiểu máy tính thật sự hoạt động thế nào.** Khi viết C bạn phải biết biến nằm ở đâu trong bộ nhớ, một chuỗi là gì ở mức byte, gọi hàm tốn gì. Những ngôn ngữ như Python hay Java che giấu các chi tiết này.
2. **Nền tảng cho ngôn ngữ khác.** C++, Objective-C, C#, Java, Go, Rust đều chịu ảnh hưởng cú pháp và tư duy của C. Biết C thì học các ngôn ngữ đó nhanh hơn.
3. **Kiểm soát tài nguyên.** C cho bạn quyết định cấp phát bao nhiêu bộ nhớ và giải phóng khi nào.
4. **Đọc được mã nguồn của những phần mềm nền tảng** khi cần tìm lỗi hoặc tìm hiểu cách chúng hoạt động.

### Cái giá phải trả

C không có "lưới an toàn". Nếu bạn ghi ra ngoài mảng, dùng bộ nhớ đã giải phóng hoặc quên khởi tạo biến, chương trình **vẫn có thể biên dịch** và chạy sai theo cách khó đoán. Vì vậy sách này luôn dạy song song hai việc: *cách viết* và *cách viết an toàn*.

## 1.2. Chương trình chạy như thế nào? (bức tranh tổng thể)

Trước khi cài công cụ, hãy hình dung bốn nhân vật:

- **Mã nguồn (source code):** file văn bản `.c` mà bạn viết. Con người đọc được.
- **Compiler (trình biên dịch):** chương trình đọc mã nguồn và tạo ra mã máy. Ví dụ: `gcc`, `clang`, `cl` (Microsoft).
- **File thực thi (executable):** kết quả cuối cùng. Trên Windows có đuôi `.exe`, trên Linux/macOS thường không có đuôi.
- **Hệ điều hành (OS):** nạp file thực thi vào bộ nhớ và giao cho CPU chạy.

```text
hello.c  ──►  [ compiler ]  ──►  hello (hoặc hello.exe)  ──►  [ hệ điều hành + CPU ]  ──►  kết quả
mã nguồn        gcc/clang          mã máy                        chạy chương trình
```

## 1.3. Cài đặt môi trường

Bạn cần ba thứ: **trình soạn thảo**, **terminal** và **compiler**.

- Trình soạn thảo: Visual Studio Code là lựa chọn phổ biến và miễn phí. Notepad++ hay Vim cũng được.
- Terminal: PowerShell/CMD (Windows), Terminal (macOS), bất kỳ terminal nào của Linux.
- Compiler: `gcc` hoặc `clang`.

### Windows — cách 1: MSYS2 + MinGW-w64 (khuyến nghị)

MinGW-w64 là bản `gcc` chạy trên Windows, dùng cùng lệnh với Linux nên hướng dẫn trong sách áp dụng trực tiếp.

1. Tải MSYS2 từ https://www.msys2.org và cài đặt (mặc định vào `C:\msys64`).
2. Mở **MSYS2 UCRT64** (hoặc MINGW64) từ Start Menu và cập nhật:

```bash
pacman -Syu
```

Nếu terminal yêu cầu đóng lại, hãy đóng, mở lại và chạy lại lệnh trên.

3. Cài toolchain (gcc, make, gdb):

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-gdb make
```

4. Thêm `C:\msys64\ucrt64\bin` vào biến môi trường `PATH` của Windows nếu bạn muốn gọi `gcc` từ PowerShell hoặc VS Code (Settings → *Edit the system environment variables* → *Path* → *New*).

### Windows — cách 2: Visual Studio

Cài Visual Studio Community, chọn workload **Desktop development with C++**. Mở **Developer Command Prompt for VS** rồi biên dịch bằng `cl hello.c`. Lưu ý: các cờ (flag) của `cl` khác `gcc`; sách này dùng cú pháp `gcc`.

### Windows — cách 3: WSL (Windows Subsystem for Linux)

Nếu bạn đã dùng WSL, cài Ubuntu rồi làm theo hướng dẫn Linux bên dưới. Đây là cách sát môi trường thực tế nhất, đặc biệt khi học các chương về thread và socket (chương 15–16).

### Linux (Debian/Ubuntu)

```bash
sudo apt update
sudo apt install build-essential gdb valgrind make
```

`build-essential` gồm `gcc`, `g++` và `make`.

### macOS

```bash
xcode-select --install     # cài clang và các công cụ dòng lệnh
brew install make gdb      # tùy chọn; macOS thường dùng lldb thay cho gdb
```

Trên macOS, lệnh `gcc` thực ra là `clang`. Điều này không ảnh hưởng gì đến sách.

### Kiểm tra cài đặt

Mở terminal và chạy:

```bash
gcc --version
make --version
gdb --version
```

Nếu thấy số phiên bản, bạn đã sẵn sàng. Nếu thấy `'gcc' is not recognized...` (Windows) hoặc `command not found` (Linux/macOS), nghĩa là compiler chưa nằm trong `PATH` — xem mục 1.10.

## 1.4. Chương trình đầu tiên: Hello, World

Tạo thư mục `c_book_practice`, tạo file `hello.c` với nội dung:

```c
#include <stdio.h>

int main(void) {
    printf("Hello, World!\n");
    return 0;
}
```

Biên dịch và chạy:

```bash
gcc -o hello hello.c
./hello          # Linux/macOS/MSYS2
hello.exe        # PowerShell/CMD trên Windows (hoặc .\hello.exe)
```

Kết quả:

```text
Hello, World!
```

### Giải thích từng dòng

**Dòng 1: `#include <stdio.h>`**

Dòng bắt đầu bằng `#` là chỉ thị cho *bộ tiền xử lý (preprocessor)*, chạy **trước** khi compiler dịch. `#include` nghĩa là "chép nội dung file này vào đây". `stdio.h` (*standard input/output header*) chứa **khai báo** của các hàm nhập/xuất như `printf`, `scanf`, `fgets`. Nếu thiếu dòng này, compiler không biết `printf` là gì và sẽ báo lỗi hoặc cảnh báo.

Dấu `< >` nghĩa là tìm file trong thư mục header của hệ thống. Dấu `" "` (ví dụ `#include "myheader.h"`) nghĩa là tìm trong thư mục dự án trước; bạn sẽ dùng cách này ở chương 11.

**Dòng 3: `int main(void) {`**

- `main` là **điểm bắt đầu** của mọi chương trình C. Khi hệ điều hành chạy chương trình, nó gọi `main`.
- `int` là **kiểu trả về**: `main` trả về một số nguyên cho hệ điều hành.
- `(void)` nghĩa là `main` không nhận tham số. (Ở chương 13 bạn sẽ thấy dạng `int main(int argc, char *argv[])` để nhận tham số dòng lệnh.)
- Dấu `{` mở **thân hàm**; dấu `}` đóng nó. Mọi lệnh nằm giữa hai dấu ngoặc là một phần của hàm.

**Dòng 4: `printf("Hello, World!\n");`**

- `printf` là hàm in ra **stdout** (đầu ra chuẩn, thường là màn hình terminal).
- `"Hello, World!\n"` là một **chuỗi ký tự (string literal)**. Trong đó `\n` là **escape sequence** biểu diễn ký tự xuống dòng. Nếu bỏ `\n`, dấu nhắc lệnh của terminal sẽ dính liền sau chữ `World!`.
- Dấu `;` kết thúc một **câu lệnh**. Thiếu `;` là lỗi cú pháp phổ biến nhất của người mới.

**Dòng 5: `return 0;`**

Trả về `0` cho hệ điều hành, quy ước là **"chạy thành công"**. Giá trị khác 0 báo hiệu có lỗi. Bạn có thể kiểm tra giá trị này ngay sau khi chạy:

```bash
./hello
echo $?          # Linux/macOS/MSYS2: in ra 0
```

```powershell
.\hello.exe
echo $LASTEXITCODE   # PowerShell: in ra 0
```

Từ C99, nếu `main` chạy tới cuối mà không có `return`, compiler tự hiểu là `return 0;`. Tuy nhiên, hãy viết rõ ràng để ý định của bạn dễ đọc.

### Các escape sequence thường dùng

| Chuỗi | Ý nghĩa |
|---|---|
| `\n` | Xuống dòng (newline) |
| `\t` | Dấu tab |
| `\\` | Một dấu gạch chéo ngược `\` |
| `\"` | Dấu nháy kép `"` bên trong chuỗi |
| `\'` | Dấu nháy đơn `'` |
| `\0` | Ký tự null, dùng để kết thúc chuỗi (chương 7) |
| `\r` | Đưa con trỏ về đầu dòng |

Ví dụ:

```c
printf("Ten:\tAn\nTuoi:\t20\n");
printf("Cô ấy nói: \"Xin chào\"\n");
```

### Một lưu ý cho Windows về tiếng Việt

Terminal của Windows mặc định không dùng UTF-8, nên chữ có dấu có thể bị hiện thành ký tự lạ. Trước khi chạy chương trình, hãy gõ:

```powershell
chcp 65001
```

Nếu vẫn lỗi, dùng Windows Terminal thay cho CMD cũ. Trong các ví dụ của sách, một số chỗ dùng tiếng Việt không dấu để chương trình chạy đúng trên mọi terminal.

## 1.5. Từ file `.c` đến file chạy được — bốn giai đoạn

Lệnh `gcc -o hello hello.c` thực ra làm bốn việc liên tiếp. Hiểu chúng giúp bạn đọc được lỗi sau này (đặc biệt lỗi *linker* ở chương 11).

```text
hello.c ─► [1 Tiền xử lý] ─► hello.i ─► [2 Biên dịch] ─► hello.s ─► [3 Hợp ngữ] ─► hello.o ─► [4 Liên kết] ─► hello
```

1. **Tiền xử lý (preprocessing):** xử lý các dòng `#`. `#include <stdio.h>` được thay bằng toàn bộ nội dung của `stdio.h`; comment bị xóa; macro được thay thế.
2. **Biên dịch (compiling):** dịch C thành **hợp ngữ (assembly)** của CPU đích.
3. **Hợp ngữ (assembling):** chuyển assembly thành **object file** (`.o`/`.obj`) chứa mã máy nhưng chưa hoàn chỉnh — nó còn "chỗ trống" cho những hàm nằm ở nơi khác (như `printf`).
4. **Liên kết (linking):** *linker* ghép object file của bạn với thư viện chuẩn của C để lấp các chỗ trống đó và tạo file thực thi.

Bạn có thể dừng ở từng giai đoạn để quan sát:

```bash
gcc -E hello.c -o hello.i     # chỉ tiền xử lý (mở hello.i, bạn sẽ thấy hàng trăm dòng của stdio.h)
gcc -S hello.c -o hello.s     # chỉ đến assembly
gcc -c hello.c -o hello.o     # chỉ đến object file
gcc hello.o -o hello          # liên kết
```

Việc này quan trọng vì **lỗi ở mỗi giai đoạn có dạng khác nhau**:

| Giai đoạn | Lỗi thường gặp | Dấu hiệu |
|---|---|---|
| Tiền xử lý | Thiếu file header | `fatal error: xyz.h: No such file or directory` |
| Biên dịch | Sai cú pháp, sai kiểu | `error: expected ';' before ...` |
| Liên kết | Gọi hàm nhưng chưa có định nghĩa | `undefined reference to 'foo'` |
| Khi chạy | Lỗi logic, truy cập bộ nhớ sai | `Segmentation fault`, kết quả sai |

## 1.6. Các cờ biên dịch nên dùng từ ngày đầu

Cách tối thiểu là `gcc -o hello hello.c`. Tuy nhiên, compiler có thể giúp bạn rất nhiều nếu bạn yêu cầu nó:

```bash
gcc -std=c11 -Wall -Wextra -g -O0 -o hello hello.c
```

| Cờ | Ý nghĩa |
|---|---|
| `-std=c11` | Dùng chuẩn C11 (một chuẩn hiện đại, được hỗ trợ rộng). Có thể dùng `c17`. |
| `-Wall` | Bật hầu hết cảnh báo hữu ích (*Warnings: all*). |
| `-Wextra` | Bật thêm các cảnh báo bổ sung. |
| `-g` | Nhúng thông tin gỡ lỗi (tên biến, số dòng) để dùng với `gdb`. |
| `-O0` | Không tối ưu hóa, giúp mã chạy đúng theo thứ tự bạn viết khi gỡ lỗi. |
| `-o tên` | Đặt tên file kết quả. Nếu bỏ, gcc tạo `a.out` (Linux/macOS) hoặc `a.exe` (Windows). |
| `-Werror` | Biến cảnh báo thành lỗi. Hữu ích khi làm dự án nghiêm túc. |

> **Quy tắc vàng:** *Cảnh báo là lỗi chưa xảy ra.* Đừng bao giờ bỏ qua cảnh báo. Nếu bạn thấy cảnh báo, hãy sửa cho đến khi biên dịch sạch.

Ví dụ, thử bỏ dòng `#include <stdio.h>` và biên dịch với `-Wall`. Bạn sẽ nhận được thông báo dạng:

```text
hello.c: In function 'main':
hello.c:2:5: warning: implicit declaration of function 'printf' [-Wimplicit-function-declaration]
    2 |     printf("Hello, World!\n");
      |     ^~~~~~
```

Cách đọc: `hello.c:2:5` nghĩa là **file hello.c, dòng 2, cột 5**. Sau đó là mức độ (`warning`/`error`), nội dung, và tên cờ gây ra cảnh báo trong ngoặc vuông. Hãy tập thói quen đọc dòng đầu tiên của thông báo lỗi trước; nhiều lỗi phía sau chỉ là hệ quả của lỗi đầu.

## 1.7. Comment (chú thích)

Comment là phần chú thích cho người đọc, compiler bỏ qua.

```c
// Comment một dòng (từ C99)

/* Comment nhiều dòng.
   Có thể trải dài trên nhiều dòng. */

int main(void) {
    int tuoi = 20;   // comment cuối dòng
    return 0;
}
```

Lưu ý: comment **không lồng nhau** được với kiểu `/* ... */`. Nếu bạn muốn "tạm tắt" một đoạn mã đã có comment, hãy dùng `#if 0 ... #endif`.

Nguyên tắc viết comment tốt: **giải thích "vì sao", đừng lặp lại "cái gì"**. Comment `// cộng 1 vào i` cho câu `i++;` là vô ích; comment `// bỏ qua dòng tiêu đề của file CSV` mới có giá trị.

## 1.8. Đọc dữ liệu từ bàn phím và in nhiều giá trị

Một chương trình chỉ có đầu ra thì hơi nhàm chán. Ví dụ dưới đây đọc một tên và chào lại:

```c
// hello_name.c
#include <stdio.h>
#include <string.h>   // cần cho strlen

int main(void) {
    char name[128];   // mảng 128 ký tự để chứa tên

    printf("Nhap ten: ");
    if (fgets(name, sizeof(name), stdin) != NULL) {
        size_t len = strlen(name);
        if (len > 0 && name[len - 1] == '\n') {
            name[len - 1] = '\0';   // xóa ký tự xuống dòng ở cuối
        }
        printf("Xin chao, %s!\n", name);
    }
    return 0;
}
```

Giải thích các phần mới (bạn chưa cần hiểu sâu, chi tiết ở các chương sau):

- `char name[128];` khai báo một **mảng** 128 ký tự. Chuỗi trong C thực chất là mảng `char` kết thúc bằng ký tự `\0` (chương 7).
- `fgets(name, sizeof(name), stdin)` đọc **tối đa** `sizeof(name) - 1` ký tự từ bàn phím (`stdin`) vào `name`, và luôn thêm `\0` ở cuối. Nó **không bao giờ ghi tràn** mảng, khác với hàm `gets` nguy hiểm đã bị loại khỏi chuẩn C11.
- `fgets` giữ lại ký tự `\n` mà người dùng gõ khi nhấn Enter, nên đoạn mã phía dưới tự cắt nó đi.
- `fgets` trả về `NULL` nếu đọc thất bại hoặc gặp hết dữ liệu (EOF). Luôn kiểm tra giá trị trả về của hàm nhập/xuất.
- `%s` trong `printf` là **định dạng (format specifier)** dành cho chuỗi.

### Bảng định dạng `printf` cơ bản

| Định dạng | Kiểu dữ liệu | Ví dụ |
|---|---|---|
| `%d` | `int` | `printf("%d", 42);` → `42` |
| `%u` | `unsigned int` | |
| `%ld` | `long` | |
| `%f` | `double` (số thực) | `printf("%f", 3.14);` → `3.140000` |
| `%.2f` | `double`, 2 chữ số thập phân | `printf("%.2f", 3.14159);` → `3.14` |
| `%c` | `char` (một ký tự) | `printf("%c", 'A');` → `A` |
| `%s` | chuỗi | `printf("%s", "abc");` → `abc` |
| `%x` | số nguyên in dạng thập lục phân | `printf("%x", 255);` → `ff` |
| `%p` | địa chỉ (con trỏ) | |
| `%zu` | `size_t` | `printf("%zu", strlen(s));` |
| `%%` | dấu `%` | |

Bạn có thể chỉ định độ rộng: `%5d` in số chiếm tối thiểu 5 ký tự (căn phải), `%-5d` căn trái, `%05d` đệm số 0.

> **Vì sao dùng `fgets` thay vì `scanf`?** `scanf("%s", name)` dừng ở dấu cách (nên "Nguyen Van A" chỉ đọc được "Nguyen") và không giới hạn độ dài nếu bạn quên viết `%127s`. `fgets` đọc cả dòng và tự bảo vệ bộ nhớ. Chương 4 sẽ nói kỹ hơn về `scanf`.

## 1.9. Một ví dụ hoàn chỉnh: máy tính bốn phép toán

```c
// arithmetic.c
#include <stdio.h>

int main(void) {
    int a, b;

    printf("Nhap hai so nguyen (cach nhau bang dau cach): ");
    if (scanf("%d %d", &a, &b) != 2) {
        fprintf(stderr, "Loi: du lieu nhap khong hop le.\n");
        return 1;                    // mã khác 0 = có lỗi
    }

    printf("%d + %d = %d\n", a, b, a + b);
    printf("%d - %d = %d\n", a, b, a - b);
    printf("%d * %d = %d\n", a, b, a * b);

    if (b == 0) {
        printf("Khong the chia cho 0.\n");
    } else {
        printf("%d / %d = %d (phan nguyen)\n", a, b, a / b);
        printf("%d %% %d = %d (phan du)\n", a, b, a % b);
        printf("%d / %d = %.2f (so thuc)\n", a, b, (double)a / b);
    }
    return 0;
}
```

Các điểm đáng chú ý:

- `&a` là **địa chỉ** của biến `a`. `scanf` cần địa chỉ để biết phải ghi giá trị vào đâu. Quên dấu `&` là lỗi kinh điển (chương 8 giải thích vì sao).
- `scanf` trả về **số mục đọc thành công**. Ở đây ta mong đợi 2; nếu khác 2 tức là người dùng nhập sai (ví dụ gõ chữ).
- `fprintf(stderr, ...)` in ra **stderr** (luồng lỗi), tách biệt với stdout. Nhờ vậy thông báo lỗi không lẫn vào kết quả khi bạn chuyển hướng đầu ra (`./prog > out.txt`).
- Phép chia hai số nguyên trong C cho **kết quả nguyên** (`7 / 2` là `3`). Để có kết quả thực, ép một toán hạng sang `double`: `(double)a / b`.
- `%%` in ra dấu `%` (vì `%` là ký tự đặc biệt trong `printf`).

Chạy thử:

```text
Nhap hai so nguyen (cach nhau bang dau cach): 7 2
7 + 2 = 9
7 - 2 = 5
7 * 2 = 14
7 / 2 = 3 (phan nguyen)
7 % 2 = 1 (phan du)
7 / 2 = 3.50 (so thuc)
```

## 1.10. Lỗi thường gặp của người mới và cách xử lý

| Triệu chứng | Nguyên nhân thường gặp | Cách xử lý |
|---|---|---|
| `'gcc' is not recognized` / `command not found` | Chưa cài compiler hoặc chưa nằm trong `PATH`. | Cài lại, hoặc thêm thư mục `bin` của compiler vào `PATH`, rồi mở terminal mới. |
| `expected ';' before '}'` | Thiếu dấu `;` ở dòng **trước** dòng được báo. | Kiểm tra dòng phía trên vị trí lỗi. |
| `implicit declaration of function` | Quên `#include` header của hàm đó. | Thêm header phù hợp (`stdio.h`, `string.h`, `stdlib.h`...). |
| `undefined reference to 'main'` | File không có hàm `main` hoặc gõ sai tên. | Kiểm tra chính tả `main`. |
| `undefined reference to 'sqrt'` | Chưa liên kết thư viện toán. | Thêm `-lm` ở cuối lệnh gcc. |
| Chương trình chạy xong, cửa sổ đóng ngay (Windows) | Chạy bằng cách nhấp đúp thay vì từ terminal. | Chạy từ terminal. |
| Chữ tiếng Việt hiện ký tự lạ | Terminal không dùng UTF-8. | `chcp 65001` hoặc dùng Windows Terminal. |
| `Segmentation fault` | Truy cập bộ nhớ không hợp lệ. | Biên dịch với `-g` và chạy `gdb` (bên dưới). |
| Kết quả `printf` không xuất hiện trước khi crash | `stdout` được đệm (buffer), chưa kịp ghi ra. | In thêm `\n` hoặc gọi `fflush(stdout);`. |

### Gỡ lỗi cơ bản với gdb

```bash
gcc -std=c11 -Wall -Wextra -g -O0 -o prog prog.c
gdb ./prog
```

Trong gdb:

```text
(gdb) run            # chạy chương trình
(gdb) backtrace      # xem chuỗi hàm gọi dẫn tới lỗi (viết tắt: bt)
(gdb) break main     # đặt điểm dừng tại đầu main
(gdb) next           # chạy một dòng, không đi vào hàm (viết tắt: n)
(gdb) step           # chạy một dòng, đi vào hàm (viết tắt: s)
(gdb) print a        # in giá trị biến a (viết tắt: p)
(gdb) quit
```

Đây là kỹ năng quan trọng nhất sau việc viết mã: **đừng đoán, hãy quan sát**.

## 1.11. Tổ chức thư mục làm việc

Từ đầu, hãy giữ thói quen mỗi chương một thư mục:

```text
c_book_practice/
├── chapter01/
│   ├── hello.c
│   ├── hello_name.c
│   └── arithmetic.c
├── chapter02/
└── ...
```

Mã mẫu của sách nằm ở thư mục `code/chapter-NN` trong repository, mỗi thư mục có README hướng dẫn biên dịch.

## 1.12. Tóm tắt

- C là ngôn ngữ biên dịch, gần phần cứng, dùng nhiều trong hệ thống và nhúng.
- Chương trình C luôn bắt đầu tại hàm `main`; giá trị `main` trả về là mã thoát cho hệ điều hành (`0` = thành công).
- `gcc` thực hiện tiền xử lý → biên dịch → hợp ngữ → liên kết. Loại lỗi cho biết lỗi xảy ra ở giai đoạn nào.
- Luôn biên dịch với `-std=c11 -Wall -Wextra -g` và xử lý mọi cảnh báo.
- Dùng `fgets` để đọc chuỗi, kiểm tra giá trị trả về của `scanf`/`fgets`.
- Phép chia hai số nguyên cho kết quả nguyên.

## 1.13. Bài tập

**Bài 1 (dễ).** Viết chương trình in ra ba dòng: tên bạn, năm sinh, quê quán. Dùng `\n` và `\t` để căn cho đẹp.

**Bài 2 (dễ).** Viết chương trình đọc tên bằng `fgets` và in `Hello, <Tên>!`.

**Bài 3 (trung bình).** Viết chương trình nhập hai số nguyên, in tổng, hiệu, tích, thương và phần dư. Xử lý trường hợp chia cho 0.

**Bài 4 (trung bình).** Viết chương trình đọc một dòng chữ và in độ dài của nó bằng `strlen`. Sau đó tự viết lại bằng vòng lặp `while` đếm đến khi gặp `'\0'` và so sánh hai kết quả.

**Bài 5 (thử thách).** Cố tình gây ra lần lượt: (a) thiếu dấu `;`, (b) thiếu `#include <stdio.h>`, (c) gọi hàm `khong_ton_tai();`. Với mỗi lỗi, ghi lại thông báo của gcc, cho biết nó xảy ra ở **giai đoạn nào** trong bốn giai đoạn của mục 1.5 và cách sửa.

Lời giải gợi ý ở [Lời giải chương 1](solutions-01.html).

## 1.14. Tài nguyên bổ sung

- Brian Kernighan & Dennis Ritchie, *The C Programming Language* (K&R) — sách kinh điển.
- https://en.cppreference.com/w/c — tài liệu tham khảo chuẩn về ngôn ngữ và thư viện C.
- `man 3 printf`, `man 3 fgets` (Linux/macOS) — trang hướng dẫn ngay trong terminal.

Mã nguồn mẫu: /code/chapter-01
