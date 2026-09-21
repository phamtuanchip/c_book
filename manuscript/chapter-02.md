# Chương 2 — Máy tính và lập trình cơ bản

Mục tiêu chương:

- Hiểu cách máy tính lưu trữ dữ liệu: bit, byte, biểu diễn số và ký tự.
- Nắm rõ bộ nhớ (stack/heap), kích thước kiểu dữ liệu và ý nghĩa của sizeof.
- Hiểu vòng đời chương trình: tiền xử lý → biên dịch → liên kết → chạy.
- Thực hành: chuyển đổi số sang nhị phân, in bảng ASCII, và dùng malloc/free.

1. Bit, byte và biểu diễn số

- Bit: đơn vị thông tin nhỏ nhất (0 hoặc 1). 1 byte = 8 bit.
- Endianness: little-endian (LSB lưu ở địa chỉ thấp) vs big-endian (MSB ở địa chỉ thấp). Thực hành: in các byte của một int để thấy thứ tự.
- Số nguyên dấu (signed) dùng two's complement để biểu diễn số âm; kích thước phụ thuộc vào nền tảng (sizeof(int)).

Ví dụ: kiểm tra sizeof trên nền tảng

#include <stdio.h>
int main(void) {
    printf("sizeof(char)=%zu, sizeof(int)=%zu, sizeof(long)=%zu\n", sizeof(char), sizeof(int), sizeof(long));
}

2. Ký tự và chuỗi

- C dùng char (thường 1 byte) để lưu ký tự; ASCII là tập ký tự cơ bản, UTF-8 là encoding phổ biến cho Unicode.
- Hàm tiện ích: isdigit(), isalpha(), isspace() (include <ctype.h>).
- Chuỗi trong C là mảng char kết thúc bởi '\0'.

3. Bộ nhớ: stack vs heap

- Stack: lưu biến cục bộ, tự động cấp phát/giải phóng khi rời scope.
- Heap: cấp phát động qua malloc/calloc/realloc; phải free thủ công.

Ví dụ phân bổ:

#include <stdlib.h>
#include <stdio.h>
int main(void) {
    int *arr = malloc(10 * sizeof(int));
    if (!arr) return 1;
    arr[0] = 42;
    free(arr);
    return 0;
}

4. Quá trình build: tiền xử lý → compile → link

- Preprocessing: mở rộng macro, include headers → tạo file tạm (.i)
- Compiling: .i → mã máy trung gian (.s) hoặc object (.o)
- Linking: nhiều .o + thư viện → executable
- Công cụ: gcc -c file.c; gcc -o prog file.o -lm

5. Assembly và mối liên hệ với C (tóm tắt)

- Compiler sinh assembly; dùng gcc -S file.c để xem.
- Hiểu stack frame: lưu return address, local vars, và cách truyền tham số (platform dependent).

6. Bài thực hành

- bin_convert.c: viết hàm chuyển số nguyên sang chuỗi nhị phân.
- ascii_table.c: in bảng 0..127 với ký tự và mô tả.
- demo_malloc.c: minh hoạ malloc/realloc/free và kiểm tra rò bộ nhớ (dùng valgrind trên Linux).

7. Lưu ý an toàn

- Tránh buffer overflow: luôn giới hạn kích thước khi đọc chuỗi.
- Sau malloc, kiểm tra con trỏ NULL.

Tài liệu tham khảo
- K&R, cppreference.com, tài liệu gcc

---

Ghi chú: thêm minh hoạ bằng hình cho endianness và ví dụ gcc -S để sinh assembly.
