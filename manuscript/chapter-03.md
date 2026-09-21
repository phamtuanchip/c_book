# Chương 3 — Lịch sử và triết lý của ngôn ngữ C

Mục tiêu chương:

- Hiểu nguồn gốc, bối cảnh phát triển và các chuẩn chính của C.
- Nhận diện điểm khác biệt quan trọng giữa các chuẩn (K&R, C89/C90, C99, C11, C17) và ảnh hưởng tới code khi di chuyển.
- Biết cách chọn cờ biên dịch để tương thích và tránh undefined behavior.

1. Tóm tắt lịch sử

- 1970s: Dennis Ritchie phát triển C tại Bell Labs để viết hệ điều hành (Unix). C ra đời từ ngôn ngữ B và ảnh hưởng bởi ASSEMBLY nhưng có tính trừu tượng hơn.
- 1978: K&R (Kernighan & Ritchie) xuất bản cuốn "The C Programming Language" — tài liệu tham khảo ban đầu.
- 1989/1990: ANSI C (C89/C90) tiêu chuẩn hóa cú pháp và thư viện chuẩn, giúp code di động hơn.
- 1999: C99 thêm nhiều tính năng hiện đại (long long, // comment, restrict, inline, VLA).
- 2011: C11 thêm atomic và multi-thread hỗ trợ; C17 sửa lỗi và ổn định.

2. Triết lý thiết kế của C

- C theo đuổi hiệu năng và kiểm soát tài nguyên: cho phép thao tác gần phần cứng.
- Đơn giản về ngữ pháp nhưng mạnh mẽ về khả năng biểu diễn.
- Thiết kế theo kiểu "một số công cụ cho lập trình viên" thay vì che giấu mọi chi tiết.

3. Chuẩn C và tương thích

- Khi viết mã để chạy trên nhiều nền tảng, luôn xác định chuẩn biên dịch: gcc -std=c11 -Wall -Wextra
- Một số tính năng mới (VLA, long long) yêu cầu chuẩn tương ứng; nếu cần hỗ trợ compiler cũ, tránh hoặc viết fallback.
- Undefined behavior: hành vi không xác định (ví dụ: dereference NULL, buffer overflow) cần tránh — compiler có thể tạo ra các tối ưu khiến lỗi khó dò.

4. Ví dụ minh họa

- compatibility_test.c: minh họa dùng long long và các macro kiểm tra kích thước kiểu.
- Dùng gcc -std=c99 compatibility_test.c -o test để nhận biết khác biệt.

5. Bài tập và thí nghiệm

- Thử biên dịch cùng file với -std=c89, -std=c99, -std=c11 và ghi lại lỗi/cảnh báo.
- Dùng gcc -S để sinh assembly và đọc phần prologue/epilogue của hàm.

6. Lời khuyên cho lập trình viên mới

- Luôn đọc cảnh báo và sửa. Cảnh báo thường chỉ ra vấn đề tiềm ẩn.
- Tránh viết mã phụ thuộc vào kích thước kiểu (ví dụ assume int = 4 bytes). Dùng các kiểu cố định như int32_t khi cần.

Tài nguyên

- K&R "The C Programming Language"
- cppreference.com và tài liệu GNU cho gcc

Ghi chú: thêm bảng so sánh tính năng giữa các chuẩn và timeline minh hoạ.
- Lưu ý rõ các ví dụ lịch sử có thể không biên dịch trên compiler mặc định; ghi cách biên dịch (cờ -std) và mục đích giáo dục.