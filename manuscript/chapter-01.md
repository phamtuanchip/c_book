# Chương 1 — Giới thiệu và chương trình C đầu tiên

Mục tiêu chương:

- Hiểu vai trò và lịch sử ngắn gọn của ngôn ngữ C.
- Thiết lập môi trường phát triển (Windows, Linux, macOS).
- Viết, biên dịch, chạy chương trình C đầu tiên.
- Giải thích chi tiết cấu trúc nguồn, hàm main, include, và cách dùng compiler flags cơ bản.

Tổng quan

Chương này hướng dẫn từ cơ bản nhất để một người mới — chưa biết lập trình — có thể:
1) chuẩn bị môi trường, 2) viết mã C đầu tiên, 3) biên dịch và chạy, và 4) đọc được các thông báo lỗi/cảnh báo cơ bản để sửa.

1. Vì sao học C?

- C là một ngôn ngữ hệ thống, gần với phần cứng, giúp hiểu sâu cách máy tính hoạt động (bộ nhớ, con trỏ, quản lý tài nguyên).
- Nhiều ngôn ngữ hiện đại dựa trên C (C++, C#, Java có nhiều ảnh hưởng), nên nắm C giúp dễ học các ngôn ngữ khác.

2. Công cụ cơ bản và chuẩn bị môi trường (chi tiết thực hiện)

- Cần: một trình soạn thảo (VSCode, Notepad++...), một terminal (PowerShell, bash), và một compiler (gcc/clang hoặc Visual Studio cl).

Windows (MSYS2 + MinGW-w64) — cách nhanh và tương thích:
1. Tải và cài MSYS2 từ https://www.msys2.org theo hướng dẫn.
2. Mở MSYS2 và cập nhật hệ thống: pacman -Syu (đóng/mở lại terminal nếu được yêu cầu).
3. Cài toolchain 64-bit: pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-make mingw-w64-x86_64-gdb
4. Mở "MSYS2 MinGW 64-bit" shell để làm việc với gcc 64-bit.
5. Kiểm tra: gcc --version, make --version, gdb --version

Windows (Visual Studio) — nếu muốn dùng cl.exe:
- Cài Visual Studio Community với workload "Desktop development with C++". Mở "Developer Command Prompt" để chạy cl.exe.

Linux (Debian/Ubuntu):
- sudo apt update && sudo apt install build-essential gdb valgrind make

macOS (Homebrew):
- brew install gcc gdb make

Lưu ý: Sau khi cài, hãy thêm đường dẫn compiler vào PATH nếu cần.

3. Cấu trúc chương trình C cơ bản — giải thích từng thành phần

Ví dụ tối thiểu (hello.c):

#include <stdio.h>

int main(void) {
    printf("Hello, World!\n");
    return 0;
}

Giải thích:
- #include <stdio.h>: đưa vào các khai báo cho hàm nhập/xuất (printf, scanf).
- int main(void): hàm main trả về int; hệ điều hành sử dụng giá trị trả về để biết trạng thái chương trình (0 = thành công).
- printf: in ra stdout; chuỗi có thể chứa escape sequences như \n (xuống dòng).

4. Biên dịch và các cờ thường dùng

- Cơ bản: gcc -o hello hello.c
- Với cảnh báo và chuẩn: gcc -std=c11 -Wall -Wextra -O0 -g -o hello hello.c
  - -std=c11: chọn chuẩn C
  - -Wall -Wextra: bật cảnh báo phổ thông
  - -O0: tắt tối ưu để dễ debug
  - -g: sinh symbol debug cho gdb

5. Viết chương trình có input: ví dụ hello_name.c

#include <stdio.h>

int main(void) {
    char name[128];
    printf("Nhập tên: ");
    if (fgets(name, sizeof(name), stdin) != NULL) {
        /* loại bỏ newline nếu có */
        size_t len = strlen(name);
        if (len > 0 && name[len-1] == '\n') name[len-1] = '\0';
        printf("Xin chào, %s!\n", name);
    }
    return 0;
}

Ghi chú an toàn:
- Tránh dùng gets(); dùng fgets() để hạn chế buffer overflow.
- Luôn kiểm tra giá trị trả về của hàm IO.

6. Ví dụ và mã nguồn trong /code/chapter-01

- hello.c — "Hello, World!"
- hello_name.c — đọc tên người dùng và in lời chào (dùng fgets)
- arithmetic.c — nhập hai số và in tổng/hiệu/tích/thương với kiểm tra chia cho 0
- input_validation.c — ví dụ kiểm tra đầu vào hợp lệ

Mỗi ví dụ có README hướng dẫn biên dịch: gcc -std=c11 -Wall -Wextra -o exe file.c

7. Bài tập thực hành (có lời giải trong /manuscript/solutions)

- Bài tập 1: Viết chương trình in "Hello, [Tên bạn]!" — dùng fgets để đọc tên.
- Bài tập 2: Viết chương trình nhập hai số nguyên, in tổng, hiệu, tích, thương (xử lý chia cho 0).
- Bài tập 3: Viết chương trình đọc một chuỗi và in độ dài (dùng strlen).

8. Lỗi thường gặp và mẹo gỡ lỗi

- Dùng -Wall -Wextra để bắt sớm lỗi khả nghi.
- Nếu chương trình crash (segfault), chạy với gdb: gdb ./prog; run; backtrace
- Kiểm tra giá trị trả về của scanf/fgets để biết input có hợp lệ hay không.

Tài nguyên bổ sung

- K&R, "The C Programming Language"
- cppreference.com: trang tham khảo các hàm thư viện chuẩn

---

Ghi chú kỹ thuật: giữ ví dụ ngắn, có comment tiếng Việt; kiểm tra các ví dụ biên dịch trên MinGW và Linux.