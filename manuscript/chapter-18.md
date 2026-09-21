# Chương 18 — Bảo mật & an toàn bộ nhớ

Mục tiêu chương:

- Nhận diện các lỗ hổng an toàn bộ nhớ phổ biến (buffer overflow, format string, integer overflow) và biện pháp phòng tránh.
- Sử dụng static và dynamic analysis để phát hiện lỗi (clang-tidy, cppcheck, ASAN).

1. Các loại lỗ hổng phổ biến

- Buffer overflow: viết quá giới hạn mảng/chuỗi → remote code execution hoặc crash.
- Format string vulnerability: printf(user_input) → attacker kiểm soát format.
- Integer overflow/underflow: tính toán kích thước bộ nhớ dẫn tới allocation sai.

2. Defense-in-depth

- Biện pháp runtime: ASLR, DEP/NX, stack canaries, PIE.
- Biện pháp code: kiểm tra bound, dùng safe APIs (snprintf, strnlen), validate input.

3. Static/dynamic analysis

- clang-tidy/cppcheck: phát hiện pattern xấu, buffer misuse.
- AddressSanitizer (ASAN): gcc/clang với -fsanitize=address để phát hiện OOB và use-after-free.

4. Secure coding guidelines

- Validate all inputs, sử dụng size_t hợp lý, tránh signed/unsigned mismatch.
- Document ownership và lifetime của buffer/objects.
- Prefer bounds-checked APIs and explicit length tracking.

5. Ví dụ thực hành

- /code/chapter-18/buffer_overflow_demo.c: có lỗi có chủ đích và phiên bản đã vá.
- /code/chapter-18/format_string_demo.c: minh họa lỗi và fix bằng format-safe call.

Bài tập

- Sửa một chương trình chứa buffer overflow; chứng minh bằng ASAN hoặc Valgrind.

Ghi chú: kèm hướng dẫn build với -fsanitize=address và cách đọc output ASAN.