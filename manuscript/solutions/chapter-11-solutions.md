# Chương 11 — Lời giải bài tập

Các bài chương này chủ yếu là **thực hành với công cụ build**; lời giải dưới dạng file mẫu và kết quả cần quan sát. Mã của dự án `geometry` nằm ở `code/chapter-11/`.

## Bài 1: tách máy tính thành `calc.h` / `calc.c` / `main.c`

Dùng ba file ở lời giải chương 4 (bài 7). Để thấy lỗi `redefinition`, đặt trong `calc.h` một định nghĩa kiểu **không có include guard** và include header hai lần:

```c
/* bad_guard.h (KHÔNG có include guard) */
typedef struct { int x; } Point;
```

```c
/* guard_demo.c */
#include "bad_guard.h"
#include "bad_guard.h"      // lỗi: "conflicting types for 'Point'" / "redefinition of typedef"
int main(void) { Point p = {1}; return p.x - 1; }
```

Thêm `#ifndef BAD_GUARD_H / #define BAD_GUARD_H / ... / #endif` vào header thì lỗi biến mất.

## Bài 2: định nghĩa biến trong header

```c
/* counter.h */
int counter = 0;                 // SAI: định nghĩa trong header
```

Hai file `.c` cùng `#include "counter.h"` sẽ tạo hai định nghĩa của `counter` → linker: `multiple definition of 'counter'`. Sửa:

```c
/* counter.h */
extern int counter;              // KHAI BÁO

// counter.c
#include "counter.h"
int counter = 0;                 // ĐỊNH NGHĨA — đúng một nơi
```

(Từ GCC 10 mặc định `-fno-common` nên lỗi hiện rõ; với bản cũ hơn, biến chưa khởi tạo có thể "lọt" qua nhờ *common symbols*.)

## Bài 3: Makefile cho dự án 3 module

Cấu trúc: `src/{math.c,utils.c,main.c}`, `include/{math_utils.h,utils.h}`. Dùng lại Makefile ở mục 11.5 (đã có quy tắc mẫu, `-MMD -MP`, `all`, `clean`, `run`). Kiểm chứng theo dõi header:

```bash
make                           # biên dịch tất cả
touch include/utils.h
make                           # chỉ biên dịch lại các .c có #include "utils.h" (nhờ file .d)
touch src/math.c
make                           # chỉ biên dịch math.c và liên kết lại
```

Nếu sửa header mà `make` không biên dịch lại gì, bạn đã quên `-include $(DEPS)` hoặc `-MMD`.

## Bài 4: mục tiêu `asan` và `release`

```make
release: CFLAGS += -O2 -DNDEBUG
release: clean all

asan: CFLAGS += -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer
asan: LDFLAGS += -fsanitize=address,undefined
asan: clean all
```

Dùng **target-specific variables** (`target: VAR += ...`) để cờ chỉ áp dụng khi đích đó được chọn (và các phụ thuộc của nó).

## Bài 5: thư viện tĩnh `libutils.a`

```bash
gcc -std=c11 -Iinclude -c src/utils.c -o build/utils.o
ar rcs build/libutils.a build/utils.o
gcc -std=c11 -Iinclude src/main.c -Lbuild -lutils -o build/app
```

Nhớ đặt `-lutils` **sau** file nguồn/`.o` cần nó. Kiểm tra nội dung thư viện: `ar t build/libutils.a`, `nm build/libutils.a`.

## Bài 6: thư viện động

```bash
gcc -fPIC -Iinclude -c src/utils.c -o build/utils.o
gcc -shared build/utils.o -o build/libutils.so
gcc -Iinclude src/main.c -Lbuild -lutils -o build/app
LD_LIBRARY_PATH=build ./build/app          # chạy được
mv build/libutils.so build/libutils.so.bak
LD_LIBRARY_PATH=build ./build/app          # error while loading shared libraries: libutils.so: cannot open shared object file
```

Lỗi này xảy ra **lúc chạy** (loader không tìm thấy `.so`), khác với lỗi liên kết lúc build.

## Bài 7: `CMakeLists.txt` tương đương

Xem `code/chapter-11/CMakeLists.txt`. Điểm cần kiểm tra: `cmake -S . -B build && cmake --build build && ctest --test-dir build`; `target_include_directories(... PUBLIC include)` để mọi target liên kết tới thư viện tự nhận đường dẫn header.

## Bài 8: `format` và `lint`

```make
SRCS_ALL := $(wildcard src/*.c include/*.h)

.PHONY: format lint
format:
	clang-format -i $(SRCS_ALL)

lint:
	cppcheck --enable=warning,style,performance --error-exitcode=1 --inline-suppr src/
	clang-tidy $(wildcard src/*.c) -- -std=c11 -Iinclude
```

`--error-exitcode=1` để `make lint` thất bại khi có phát hiện — cần cho CI.
