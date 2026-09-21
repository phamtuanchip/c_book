# Chương 11 — Header, Makefile, build systems

Mục tiêu chương:

- Biết tổ chức dự án C nhiều file: header an toàn, tách interface/implementation.
- Viết Makefile cơ bản và giới thiệu CMake để build cross-platform.

1. Header design

- Include guard:
#ifndef MYLIB_H
#define MYLIB_H
...
#endif
- Quy tắc include order: corresponding header first, then <...>, then project headers.
- Forward declaration để tránh include vòng.

2. Makefile cơ bản

- Variables: CC = gcc, CFLAGS = -std=c11 -Wall -Wextra -g
- Targets: all, clean, test
- Pattern rule: %.o: %.c
- Ví dụ Makefile cho project 3 module: compile .c -> .o; link các .o

3. CMake cơ bản

- CMakeLists.txt đơn giản: project(name C); add_executable(); target_include_directories();
- Lợi ích: tạo build dir, hỗ trợ nhiều generator, dễ tích hợp CI

4. Best practices

- Separate build directory (out-of-source build).
- Giữ header minimal; tránh include heavy headers trong header.
- Document API trong header comments.

Bài tập

- Viết Makefile cho project có 3 module (math, utils, main) và chạy build/test; tạo CMakeLists.txt tương đương.

Ghi chú: kèm sample Makefile và CMakeLists trong /code/chapter-11 để người học chạy.