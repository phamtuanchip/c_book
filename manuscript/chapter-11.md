# Chương 11 — Header, Makefile, build systems

## Mục tiêu chương

- Tổ chức dự án C nhiều file: tách **giao diện (header)** khỏi **cài đặt (source)**.
- Hiểu **khai báo** so với **định nghĩa**, và vì sao header chỉ nên chứa khai báo.
- Hiểu quá trình biên dịch riêng từng file (`.o`) rồi liên kết, cùng các lỗi *linker* thường gặp.
- Viết `Makefile` có biến, quy tắc mẫu, phụ thuộc tự động (`-MMD`), target `all`/`clean`/`test`.
- Biết dùng **CMake** cho dự án đa nền tảng.
- Tạo và dùng **thư viện tĩnh** (`.a`) và **thư viện động** (`.so`/`.dll`).

## 11.1. Vì sao phải chia nhiều file?

Chương trình nhỏ nằm trong một file `main.c` không sao. Nhưng khi lớn lên:

- File dài hàng nghìn dòng khó đọc, khó tìm.
- Mọi thay đổi buộc biên dịch lại tất cả.
- Nhiều người cùng sửa một file dễ xung đột.
- Không tái sử dụng được một phần cho dự án khác.

Giải pháp: chia thành **module**. Mỗi module có:

- **Header `.h`** — *giao diện* công khai: prototype hàm, kiểu, hằng số mà module cho phép bên ngoài dùng.
- **Source `.c`** — *cài đặt*: định nghĩa hàm và dữ liệu nội bộ.

Người dùng module chỉ cần `#include` header, không cần biết cài đặt.

## 11.2. Khai báo và định nghĩa

| | Khai báo (declaration) | Định nghĩa (definition) |
|---|---|---|
| Ý nghĩa | "Có một thứ tên X kiểu Y tồn tại ở đâu đó" | "Đây là X, và đây là bộ nhớ/mã của nó" |
| Hàm | `int add(int a, int b);` | `int add(int a, int b) { return a + b; }` |
| Biến | `extern int counter;` | `int counter = 0;` |
| Cấp bộ nhớ? | Không | Có |
| Được lặp lại? | Nhiều lần, nhiều file | **Đúng một lần** trong cả chương trình |

Quy tắc **một định nghĩa (One Definition Rule)**: mỗi hàm và biến toàn cục phải được định nghĩa đúng **một lần** trong toàn bộ chương trình. Vì `#include` chép nguyên văn nội dung header vào mọi file `.c` nào include nó, header chứa **định nghĩa** sẽ tạo ra định nghĩa lặp → lỗi liên kết `multiple definition of ...`.

Vì thế, **header chỉ nên chứa:**

- Prototype hàm.
- Khai báo `extern` biến toàn cục (nếu thật sự cần).
- Định nghĩa kiểu: `struct`, `enum`, `typedef`, `union`.
- Hằng số và macro.
- Hàm `static inline` nhỏ (mỗi file `.c` có bản riêng nên không xung đột).

**Không đặt** trong header: thân hàm thường, biến toàn cục có khởi tạo.

## 11.3. Thiết kế một header tốt

```c
// include/geometry.h
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stddef.h>          // size_t: header này cần tự include những gì nó dùng

typedef struct {
    double x, y;
} Point;

typedef struct {
    Point *pts;
    size_t count;
} Polygon;

/* Khoảng cách Euclid giữa a và b. */
double geo_distance(Point a, Point b);

/*
 * Diện tích đa giác theo công thức Shoelace.
 * Trả về 0 nếu poly có ít hơn 3 đỉnh.
 */
double geo_polygon_area(const Polygon *poly);

#endif /* GEOMETRY_H */
```

Những nguyên tắc rút ra:

1. **Include guard** (`#ifndef ... #define ... #endif`): tránh bị include hai lần gây lỗi trùng định nghĩa kiểu. Tên guard nên duy nhất, thường theo `TÊN_FILE_H` (hoặc thêm tên dự án).
2. **Header tự đủ (self-contained):** include header đó **một mình** trong file `.c` trống phải biên dịch được. Nếu header dùng `size_t`, nó phải tự `#include <stddef.h>`.
3. **Tối thiểu hóa include:** chỉ include cái gì thật sự cần trong header; include còn lại đặt trong `.c` để giảm phụ thuộc và thời gian biên dịch.
4. **Đặt tiền tố** tên hàm theo module (`geo_`, `vec_`, `net_`) để tránh trùng tên vì C không có namespace.
5. **Ghi chú hợp đồng** ngay trên prototype: hàm làm gì, tham số nào có thể NULL, ai giải phóng bộ nhớ, giá trị trả về khi lỗi.
6. **Dùng `const`** cho tham số con trỏ chỉ đọc.

### Thứ tự `#include` gợi ý

Trong `geometry.c`:

```c
#include "geometry.h"      // 1. header tương ứng đặt ĐẦU TIÊN: kiểm chứng nó tự đủ
#include <math.h>          // 2. header hệ thống/thư viện chuẩn
#include <stdlib.h>
#include "util.h"          // 3. header khác của dự án
```

Đặt header tương ứng đầu tiên là mẹo hay: nếu header thiếu `#include` nào đó, lỗi lộ ra ngay thay vì bị "che" bởi các include phía trước.

### Khai báo chuyển tiếp (forward declaration) và include vòng

Nếu `a.h` include `b.h` và `b.h` include `a.h`, bạn gặp **include vòng**. Khi một header chỉ dùng **con trỏ** tới kiểu khác, có thể chỉ cần khai báo chuyển tiếp thay vì include:

```c
// parser.h
struct Lexer;                          // forward declaration: "có struct Lexer, chi tiết ở nơi khác"

typedef struct Parser {
    struct Lexer *lexer;               // chỉ là con trỏ nên không cần biết kích thước Lexer
    int errors;
} Parser;
```

Nhờ vậy `parser.h` không cần `#include "lexer.h"`.

### `extern "C"` cho tương thích C++

Nếu thư viện của bạn có thể được gọi từ C++, thêm:

```c
#ifdef __cplusplus
extern "C" {
#endif

/* ... prototype ... */

#ifdef __cplusplus
}
#endif
```

## 11.4. Biên dịch nhiều file — chi tiết

Với ba file:

```text
project/
├── include/geometry.h
├── src/geometry.c
└── src/main.c
```

```c
// src/geometry.c
#include "geometry.h"
#include <math.h>

double geo_distance(Point a, Point b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

double geo_polygon_area(const Polygon *poly) {
    if (poly == NULL || poly->count < 3) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < poly->count; i++) {
        Point p = poly->pts[i];
        Point q = poly->pts[(i + 1) % poly->count];
        sum += p.x * q.y - q.x * p.y;
    }
    return fabs(sum) / 2.0;
}
```

```c
// src/main.c
#include <stdio.h>
#include "geometry.h"

int main(void) {
    Point square[] = {{0, 0}, {4, 0}, {4, 3}, {0, 3}};
    Polygon poly = { square, 4 };
    printf("dien tich = %.2f\n", geo_polygon_area(&poly));           // 12.00
    printf("khoang cach = %.2f\n", geo_distance(square[0], square[2]));   // 5.00
    return 0;
}
```

Biên dịch **từng file thành object file** rồi **liên kết**:

```bash
gcc -std=c11 -Wall -Wextra -g -Iinclude -c src/geometry.c -o build/geometry.o
gcc -std=c11 -Wall -Wextra -g -Iinclude -c src/main.c     -o build/main.o
gcc build/geometry.o build/main.o -o build/app -lm
```

- `-c`: chỉ biên dịch tới `.o`, không liên kết.
- `-Iinclude`: thêm thư mục `include` vào đường tìm header (cho `#include "geometry.h"`).
- `-lm`: liên kết thư viện toán (`libm`) — **đặt sau** các file `.o` cần nó. Thứ tự đối số của linker quan trọng: thư viện phải đứng **sau** các file object dùng nó.

Lợi ích: sửa `main.c` chỉ cần biên dịch lại `main.o` rồi liên kết lại, không phải biên dịch `geometry.c`. Với dự án hàng nghìn file, điều này tiết kiệm rất nhiều thời gian — và đó chính là việc mà `make` tự động làm.

### Các lỗi liên kết (linker) thường gặp

| Thông báo | Nguyên nhân | Cách sửa |
|---|---|---|
| `undefined reference to 'geo_distance'` | Khai báo có nhưng không có định nghĩa được liên kết (quên file `.c`/`.o`, hoặc gõ sai tên) | Thêm file vào lệnh liên kết; kiểm tra chính tả và chữ ký |
| `undefined reference to 'sqrt'` | Chưa liên kết thư viện | Thêm `-lm` |
| `multiple definition of 'counter'` | Định nghĩa biến/hàm trong header, hoặc hai `.c` cùng định nghĩa | Chuyển định nghĩa sang một `.c`; header dùng `extern`/prototype |
| `fatal error: geometry.h: No such file or directory` | Thiếu `-I` | Thêm `-Iinclude` |
| `redefinition of 'struct Point'` | Header include hai lần, thiếu guard | Thêm include guard |
| `undefined reference to 'main'` | Không có hàm `main` | Đảm bảo có đúng một `main` |

Phân biệt: `No such file` là lỗi **tiền xử lý**; `undefined reference` là lỗi **linker**, xảy ra sau khi biên dịch xong tất cả file.

## 11.5. Makefile

`make` đọc file tên `Makefile`, mô tả **cái gì phụ thuộc cái gì** và **cách tạo ra nó**. Khi bạn chạy `make`, nó so sánh **thời gian sửa** của file nguồn và file đích, và chỉ chạy lệnh cho những đích **lỗi thời**.

> **Windows:** dùng `make` trong MSYS2 (gõ `make` trong shell MSYS2/MinGW, hoặc `mingw32-make` từ `mingw-w64-x86_64-make`), hoặc chạy trong WSL.

### Quy tắc cơ bản

```make
target: dependencies
<TAB>command
```

**Quan trọng:** dòng lệnh phải bắt đầu bằng **ký tự TAB**, không phải dấu cách. Đây là lỗi kinh điển: `Makefile:5: *** missing separator. Stop.`

```make
# Makefile tối giản
app: main.o geometry.o
	gcc main.o geometry.o -o app -lm

main.o: src/main.c include/geometry.h
	gcc -std=c11 -Wall -Wextra -Iinclude -c src/main.c

geometry.o: src/geometry.c include/geometry.h
	gcc -std=c11 -Wall -Wextra -Iinclude -c src/geometry.c
```

Chạy `make` → tạo `app`. Chạy lại lần nữa → "`make: 'app' is up to date.`". Sửa `geometry.c` rồi `make` → chỉ biên dịch lại `geometry.o` và liên kết lại.

### Makefile hoàn chỉnh, đầy đủ

```make
# ---- Cấu hình ----
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g -O0 -Iinclude
LDFLAGS :=
LDLIBS  := -lm

SRC_DIR   := src
BUILD_DIR := build
TARGET    := $(BUILD_DIR)/app

SRCS := $(wildcard $(SRC_DIR)/*.c)                     # mọi file .c trong src/
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)                                  # file phụ thuộc tự sinh

# ---- Mục tiêu ----
.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Quy tắc mẫu: mỗi src/X.c -> build/X.o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./tests/run_tests.sh

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
```

Giải thích từng phần:

- **Biến:** `CC := gcc`; dùng bằng `$(CC)`. `:=` gán ngay lập tức. Đặt cờ ở đầu để dễ đổi (`make CC=clang`, `make CFLAGS="-O2"`).
- **`$(wildcard ...)`** liệt kê file; **`$(patsubst ...)`** biến đổi tên (`src/a.c` → `build/a.o`).
- **Biến tự động** trong quy tắc:
  - `$@` — tên **đích** (target).
  - `$<` — **phụ thuộc đầu tiên** (thường là file `.c`).
  - `$^` — **tất cả** phụ thuộc.
- **Quy tắc mẫu `%`:** `$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c` áp dụng cho mọi file — không phải viết từng quy tắc.
- **`| $(BUILD_DIR)`:** phụ thuộc "order-only": đảm bảo thư mục tồn tại trước, nhưng thay đổi thư mục không kích hoạt biên dịch lại.
- **`.PHONY`:** khai báo các đích không phải file thật (`all`, `clean`...) để `make` luôn chạy chúng, kể cả khi vô tình tồn tại file tên `clean`.
- **`-MMD -MP`:** yêu cầu gcc sinh file `.d` liệt kê **các header** mà mỗi `.c` include. **`-include $(DEPS)`** nạp chúng, nên khi bạn sửa `geometry.h`, `make` tự biên dịch lại mọi `.c` phụ thuộc. Nếu thiếu bước này, sửa header sẽ **không** kích hoạt biên dịch lại — nguồn lỗi khó chịu (bạn thay đổi `struct` mà file cũ vẫn dùng bố cục cũ).
- Dấu `@` đầu lệnh (`@echo ...`) ẩn dòng lệnh khi chạy; `-` đầu lệnh bỏ qua lỗi.

### Các mục tiêu thường thấy

```bash
make            # biên dịch mọi thứ (đích mặc định: đích đầu tiên, thường là all)
make clean      # xóa file đã sinh
make run        # biên dịch và chạy
make -j4        # biên dịch song song 4 tiến trình
make V=1 ...    # (tùy Makefile) hiện đầy đủ dòng lệnh
```

### Các biến thể build

```make
# Đặt bản debug và release
ifeq ($(BUILD),release)
    CFLAGS += -O2 -DNDEBUG
else
    CFLAGS += -O0 -g
endif

# make BUILD=release
```

Bản **sanitizer**:

```make
asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
asan: LDFLAGS += -fsanitize=address,undefined
asan: clean all
```

## 11.6. Thư viện tĩnh và thư viện động

### Thư viện tĩnh (`.a`)

Gói nhiều file `.o` thành một file lưu trữ. Khi liên kết, linker **chép** phần mã cần dùng vào file thực thi.

```bash
gcc -c -Iinclude src/geometry.c -o build/geometry.o
ar rcs build/libgeometry.a build/geometry.o            # tạo thư viện tĩnh
gcc src/main.c -Iinclude -Lbuild -lgeometry -lm -o app # -L: thư mục tìm; -lgeometry: tìm libgeometry.a
```

Quy ước đặt tên: `libTÊN.a` được nhắc tới bằng `-lTÊN`. Ưu điểm: file chạy độc lập, không phụ thuộc file khác lúc chạy. Nhược điểm: file lớn hơn; cập nhật thư viện phải liên kết lại.

### Thư viện động (`.so` trên Linux, `.dll` trên Windows, `.dylib` trên macOS)

Mã **không** được chép vào file thực thi; nó được nạp lúc chạy và có thể dùng chung giữa nhiều chương trình.

```bash
# Linux
gcc -fPIC -c -Iinclude src/geometry.c -o build/geometry.o    # -fPIC: mã không phụ thuộc vị trí
gcc -shared build/geometry.o -o build/libgeometry.so
gcc src/main.c -Iinclude -Lbuild -lgeometry -lm -o app
LD_LIBRARY_PATH=build ./app                                   # chỉ cho hệ thống biết chỗ tìm .so lúc chạy
```

Trên Windows/MinGW: `gcc -shared -o geometry.dll geometry.o -Wl,--out-implib,libgeometry.a`. Lỗi thường gặp lúc chạy: `error while loading shared libraries: libgeometry.so: cannot open shared object file` — hệ thống không tìm thấy `.so` (đặt `LD_LIBRARY_PATH`, dùng `rpath` `-Wl,-rpath,'$ORIGIN'`, hoặc cài vào thư mục hệ thống).

## 11.7. CMake

**CMake** là trình **tạo hệ thống build** (không tự biên dịch): từ một file mô tả `CMakeLists.txt`, nó sinh ra Makefile, dự án Ninja, Visual Studio, Xcode... — phù hợp dự án cần chạy trên nhiều nền tảng.

Cài đặt: `sudo apt install cmake`, `brew install cmake`, hoặc tải từ cmake.org (Windows).

### `CMakeLists.txt` cho dự án ở trên

```cmake
cmake_minimum_required(VERSION 3.16)
project(geometry_demo VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)                 # -std=c11 thay vì gnu11

# Thư viện tĩnh từ geometry.c
add_library(geometry STATIC src/geometry.c)
target_include_directories(geometry PUBLIC include)
target_compile_options(geometry PRIVATE -Wall -Wextra -Wpedantic)
find_library(MATH_LIB m)                    # libm (trên Windows/MSVC không cần)
if(MATH_LIB)
    target_link_libraries(geometry PUBLIC ${MATH_LIB})
endif()

# Chương trình chính
add_executable(app src/main.c)
target_link_libraries(app PRIVATE geometry)

# Kiểm thử (ctest)
enable_testing()
add_test(NAME smoke COMMAND app)
```

### Quy trình dùng

```bash
cmake -S . -B build                   # cấu hình: sinh build system vào thư mục build/
cmake --build build                   # biên dịch (dùng generator đã chọn)
ctest --test-dir build                # chạy kiểm thử
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release     # bản release
cmake -S . -B build -G Ninja          # dùng Ninja (nhanh hơn make)
```

Các điểm mấu chốt của CMake hiện đại ("target-based"):

- Mỗi thứ là một **target** (`add_library`, `add_executable`).
- Thông tin gắn vào target bằng `target_include_directories`, `target_compile_options`, `target_link_libraries`.
- Phạm vi `PUBLIC`/`PRIVATE`/`INTERFACE` cho biết thông tin có truyền sang target phụ thuộc hay không (ví dụ thư mục `include` của `geometry` là `PUBLIC` nên `app` tự nhận nó khi liên kết).
- **Out-of-source build:** luôn build trong thư mục riêng (`build/`), không lẫn file sinh ra với mã nguồn; xóa nhanh bằng cách xóa thư mục đó.

### Make hay CMake?

| | Make | CMake |
|---|---|---|
| Độ phức tạp | Thấp, trực tiếp | Cao hơn, thêm một lớp |
| Đa nền tảng | Khó (phụ thuộc shell/công cụ) | Rất tốt (VS, Xcode, Ninja...) |
| Tìm thư viện | Tự làm (`pkg-config`) | `find_package`, `FetchContent` |
| Phù hợp | Dự án nhỏ/vừa, Linux/macOS | Dự án đa nền tảng, có phụ thuộc |

Hãy bắt đầu bằng `make`; chuyển sang CMake khi cần Windows/Visual Studio hoặc nhiều phụ thuộc.

## 11.8. Thực hành tốt

1. **Build ngoài mã nguồn:** tất cả file sinh ra vào `build/`, thêm `build/` vào `.gitignore`.
2. **Luôn bật cảnh báo** trong Makefile/CMake (`-Wall -Wextra`); cân nhắc `-Werror` trong CI.
3. **Tách cờ debug/release/sanitizer** bằng biến hoặc mục tiêu riêng.
4. **Giữ header nhỏ và tự đủ;** chỉ lộ những gì cần lộ. Mọi thứ khác đánh dấu `static` trong `.c`.
5. **Tự động hóa kiểm thử:** `make test` hoặc `ctest` chạy cả bộ kiểm thử (chương 21).
6. **Ghi cách build vào README** (các lệnh chính xác).
7. **Không commit file build** (`*.o`, `*.exe`, thư mục `build/`); chỉ commit mã nguồn và file cấu hình.

## 11.9. Tóm tắt

- Header chứa **khai báo**, source chứa **định nghĩa**; mỗi định nghĩa chỉ một lần.
- Include guard, header tự đủ, tiền tố tên, forward declaration giúp header bền vững.
- Biên dịch từng file (`-c`) rồi liên kết; đọc lỗi linker để phân biệt với lỗi biên dịch.
- Makefile: mục tiêu, phụ thuộc, biến tự động, quy tắc mẫu, `-MMD` để theo dõi header.
- Thư viện tĩnh `.a` (chép vào chương trình) và động `.so`/`.dll` (nạp lúc chạy).
- CMake mô tả dự án theo target và sinh hệ thống build cho mọi nền tảng.

## 11.10. Bài tập

1. Tách chương trình máy tính (chương 4) thành `calc.h`/`calc.c`/`main.c`. Cố tình xóa include guard rồi include header hai lần để thấy lỗi `redefinition`.
2. Cố tình đặt **định nghĩa** biến `int counter = 0;` trong header, include vào hai file `.c` và quan sát lỗi `multiple definition`. Sửa bằng `extern`.
3. Viết Makefile cho dự án 3 module (`math`, `utils`, `main`) có `all`, `clean`, `run`, dùng quy tắc mẫu và `-MMD`. Kiểm chứng: sửa một header và xác nhận chỉ các file phụ thuộc được biên dịch lại.
4. Thêm mục tiêu `asan` và `release` vào Makefile của bạn.
5. Tạo thư viện tĩnh `libutils.a` từ các module và liên kết chương trình chính với nó bằng `-L` và `-l`.
6. Tạo thư viện động và chạy chương trình dùng nó; thử xóa/đổi tên `.so` để thấy lỗi nạp lúc chạy.
7. Viết `CMakeLists.txt` tương đương với Makefile ở bài 3; build bằng `cmake -S . -B build` và chạy `ctest`.
8. (Thử thách) Thêm mục tiêu `format` (chạy `clang-format`) và `lint` (chạy `cppcheck`) vào Makefile.

Mã nguồn mẫu: /code/chapter-11
