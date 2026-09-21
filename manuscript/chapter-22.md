# Chương 22 — Packaging & deploy

## Mục tiêu chương

- Chuẩn bị một dự án C để **phát hành**: đánh phiên bản, cấu trúc thư mục, giấy phép, tài liệu.
- Viết mục tiêu **`install`/`uninstall`/`dist`** trong Makefile (hỗ trợ `PREFIX` và `DESTDIR`) và cài đặt qua CMake.
- Tạo **gói nguồn** (`tar.gz`, `zip`) và **gói nhị phân**; tạo **checksum** và chữ ký.
- Hiểu **biên dịch chéo (cross-compilation)**: MinGW-w64 (Windows từ Linux), toolchain ARM, `musl` cho nhị phân Linux tĩnh.
- Phân biệt liên kết **tĩnh** và **động**, kiểm tra phụ thuộc bằng `ldd`/`otool`/`objdump`; hiểu vấn đề tương thích `glibc`.
- Tự động hóa phát hành bằng **GitHub Actions + GitHub Releases** khi gắn tag.
- Đóng gói bằng **Docker (multi-stage)**, `.deb`/`.rpm` (CPack/fpm), và biết các kênh phân phối khác (Homebrew, AUR, vcpkg/Conan).
- Áp dụng cho chính cuốn sách này: quy trình sinh EPUB/PDF và chuẩn bị hồ sơ **Amazon KDP**.

## 22.1. Phát hành phần mềm nghĩa là gì?

Mã chạy được trên máy bạn chưa phải là **sản phẩm**. Người dùng cần:

1. **Một cách lấy phần mềm** (tải file, cài qua trình quản lý gói, container).
2. **Biết đó là phiên bản nào** và có thay đổi gì.
3. **Yên tâm về nguồn gốc** (checksum, chữ ký).
4. **Chạy được trên hệ thống của họ** (kiến trúc, phụ thuộc, phiên bản thư viện).
5. **Có tài liệu và giấy phép** rõ ràng.

Quy trình đề xuất: **đánh phiên bản → build sạch có thể lặp lại → kiểm thử → đóng gói → ký/checksum → công bố → ghi lại thay đổi**. Chương này đi qua từng bước.

## 22.2. Chuẩn bị dự án

### Cấu trúc thư mục điển hình

```text
myapp/
├── LICENSE                 # giấy phép (MIT, Apache-2.0, GPL...)
├── README.md               # giới thiệu, cách build, cách dùng
├── CHANGELOG.md            # lịch sử thay đổi theo phiên bản
├── VERSION                 # (tùy chọn) số phiên bản một dòng: 1.2.0
├── Makefile
├── CMakeLists.txt          # (tùy chọn)
├── include/myapp/          # header công khai
├── src/                    # mã nguồn
├── tests/
├── docs/                   # tài liệu, man page (myapp.1)
└── scripts/                # script build/phát hành
```

### Đánh phiên bản theo Semantic Versioning (SemVer)

Định dạng **`MAJOR.MINOR.PATCH`** (ví dụ `1.4.2`):

| Thay đổi | Tăng | Ví dụ |
|---|---|---|
| Sửa lỗi, tương thích ngược | **PATCH** | `1.4.2` → `1.4.3` |
| Thêm tính năng, tương thích ngược | **MINOR** | `1.4.3` → `1.5.0` |
| Thay đổi phá vỡ tương thích (API/ABI) | **MAJOR** | `1.5.0` → `2.0.0` |

Phiên bản trước khi ổn định: `0.y.z`. Phiên bản thử: `1.0.0-rc.1`.

**Nhúng phiên bản vào chương trình** (một nguồn duy nhất, không sao chép thủ công):

```make
VERSION := $(shell cat VERSION)
CFLAGS  += -DAPP_VERSION=\"$(VERSION)\"
```

```c
#ifndef APP_VERSION
#define APP_VERSION "dev"
#endif

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        printf("myapp %s\n", APP_VERSION);
        return 0;
    }
    /* ... */
}
```

Bổ sung thông tin commit: `-DAPP_COMMIT=\"$(shell git rev-parse --short HEAD 2>/dev/null || echo unknown)\"`. Lỗi nhận từ người dùng kèm `myapp --version` sẽ cho bạn biết chính xác bản nào.

### Giấy phép

Không có giấy phép, mã **mặc định là "mọi quyền được bảo lưu"** — người khác không được phép dùng. Chọn một giấy phép và đặt file `LICENSE`:

| Giấy phép | Tính chất |
|---|---|
| **MIT / BSD** | Rất thoáng, cho dùng cả trong sản phẩm đóng; chỉ cần giữ thông báo bản quyền |
| **Apache-2.0** | Như MIT + cấp phép bằng sáng chế rõ ràng |
| **GPL-3.0** | "Copyleft": sản phẩm phái sinh phải cũng mở nguồn theo GPL |
| **LGPL** | Copyleft yếu cho thư viện: được liên kết từ phần mềm đóng nếu liên kết động |

Nếu bạn dùng thư viện bên thứ ba, kiểm tra **giấy phép của chúng có tương thích** với giấy phép của bạn không, và giữ các thông báo bản quyền cần thiết (file `THIRD_PARTY_NOTICES`).

### CHANGELOG

Theo định dạng "Keep a Changelog": mỗi phiên bản có mục `Added`, `Changed`, `Fixed`, `Removed`, `Security`:

```markdown
## [1.2.0] - 2026-03-15
### Added
- Tùy chọn `--json` cho đầu ra.
### Fixed
- Rò rỉ bộ nhớ khi đọc file rỗng (#42).
```

## 22.3. Cài đặt: `PREFIX`, `DESTDIR` và `install` trong Makefile

Người dùng nguồn quen với `make && sudo make install`. Quy ước:

- **`PREFIX`** (mặc định `/usr/local`): nơi cài **khi chạy thật**.
- **`DESTDIR`** (mặc định rỗng): thư mục **giả lập gốc** khi *đóng gói* — file được chép vào `$(DESTDIR)$(PREFIX)/...` nhưng đường dẫn nhúng trong chương trình vẫn dùng `PREFIX`. Người đóng gói `.deb`/`.rpm` dùng `make install DESTDIR=/tmp/pkgroot` để gom file mà không cần quyền root.

```make
NAME     := myapp
VERSION  := $(shell cat VERSION)
PREFIX   ?= /usr/local
DESTDIR  ?=
BINDIR   := $(PREFIX)/bin
MANDIR   := $(PREFIX)/share/man/man1
DOCDIR   := $(PREFIX)/share/doc/$(NAME)

CC       ?= gcc
CFLAGS   ?= -O2
CFLAGS   += -std=c11 -Wall -Wextra -DAPP_VERSION=\"$(VERSION)\"

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=build/%.o)

.PHONY: all install uninstall dist clean

all: build/$(NAME)

build/$(NAME): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

build:
	mkdir -p build

install: all
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(MANDIR) $(DESTDIR)$(DOCDIR)
	install -m 0755 build/$(NAME)      $(DESTDIR)$(BINDIR)/$(NAME)
	install -m 0644 docs/$(NAME).1     $(DESTDIR)$(MANDIR)/$(NAME).1
	install -m 0644 README.md LICENSE  $(DESTDIR)$(DOCDIR)/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(NAME) $(DESTDIR)$(MANDIR)/$(NAME).1
	rm -rf $(DESTDIR)$(DOCDIR)

clean:
	rm -rf build dist
```

Lệnh `install` (công cụ, khác mục tiêu Make) tạo thư mục và đặt quyền đúng (`-m 0755` chạy được, `0644` chỉ đọc). Dùng thử:

```bash
make PREFIX=$HOME/.local install        # cài cho riêng người dùng, không cần sudo
make DESTDIR=/tmp/pkgroot install       # gom file để đóng gói, kiểm tra bằng: find /tmp/pkgroot -type f
```

Tôn trọng biến môi trường `CC`, `CFLAGS`, `LDFLAGS` (viết `?=` và `+=`) để người đóng gói của các bản phân phối (Debian, Fedora, Homebrew) truyền cờ của họ.

### Cài đặt bằng CMake

```cmake
include(GNUInstallDirs)                       # CMAKE_INSTALL_BINDIR = bin, ...

install(TARGETS myapp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES docs/myapp.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})
```

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
DESTDIR=/tmp/pkgroot cmake --install build
```

### Thư viện: header công khai và `pkg-config`

Nếu dự án là **thư viện**, hãy cài thêm header, file `.a`/`.so`, và tệp mô tả `pkg-config` để người khác dùng `pkg-config --cflags --libs mylib`:

```text
# mylib.pc
prefix=/usr/local
libdir=${prefix}/lib
includedir=${prefix}/include

Name: mylib
Description: Thu vien vi du
Version: 1.2.0
Libs: -L${libdir} -lmylib
Cflags: -I${includedir}
```

Với thư viện dùng chung `.so`, quản lý **SONAME** và phiên bản ABI (`libmylib.so.1.2.0` → `libmylib.so.1` → `libmylib.so`); chỉ tăng MAJOR khi phá vỡ ABI.

## 22.4. Gói nguồn: `make dist`, tarball và zip

Gói nguồn cho phép người khác tự build. Nên chứa **chỉ mã nguồn, tài liệu, file build** — không chứa `build/`, `.git`, file tạm.

### Cách đáng tin cậy: `git archive`

```make
DISTNAME := $(NAME)-$(VERSION)

dist:
	mkdir -p dist
	git archive --format=tar.gz --prefix=$(DISTNAME)/ -o dist/$(DISTNAME).tar.gz HEAD
	git archive --format=zip    --prefix=$(DISTNAME)/ -o dist/$(DISTNAME).zip    HEAD
	cd dist && sha256sum $(DISTNAME).tar.gz $(DISTNAME).zip > SHA256SUMS
```

- `--prefix=$(DISTNAME)/` để khi giải nén, mọi thứ nằm trong thư mục `myapp-1.2.0/` (không "xả" tung tóe vào thư mục hiện tại).
- `git archive` chỉ lấy những file **đã được commit**, tôn trọng `.gitattributes` (`export-ignore` để loại `tests/data` lớn, `.github`).
- Dùng khi đã **gắn tag** phiên bản: `git archive ... v1.2.0`.

Không dùng git? Có thể dùng `tar` thủ công:

```bash
tar --exclude='./build' --exclude='./.git' --exclude='./dist' \
    --transform "s,^\.,myapp-1.2.0," -czf dist/myapp-1.2.0.tar.gz .
```

### Kiểm tra gói nguồn xây được từ đầu

Sai lầm phổ biến: quên đưa một file vào gói, và chỉ phát hiện khi người dùng báo. Kiểm thử:

```bash
rm -rf /tmp/verify && mkdir /tmp/verify && cd /tmp/verify
tar xzf ~/myapp/dist/myapp-1.2.0.tar.gz
cd myapp-1.2.0 && make && make test && make DESTDIR=/tmp/verify/root install
```

Nếu thành công trong môi trường sạch, gói nguồn đầy đủ.

### Checksum

```bash
sha256sum myapp-1.2.0.tar.gz > SHA256SUMS      # Linux
shasum -a 256 myapp-1.2.0.tar.gz > SHA256SUMS  # macOS
# Người dùng kiểm tra:
sha256sum -c SHA256SUMS
```

Checksum chứng minh file không **bị hỏng khi tải** — nhưng nếu kẻ tấn công thay được file thì cũng thay được checksum cùng chỗ; muốn chống giả mạo cần **chữ ký** (mục 22.9).

## 22.5. Liên kết tĩnh và động — chọn cái nào để phân phối?

| | Liên kết **động** (mặc định) | Liên kết **tĩnh** |
|---|---|---|
| Thư viện | Nạp lúc chạy (`.so`, `.dll`, `.dylib`) | Chép vào file thực thi (`.a`) |
| Kích thước file | Nhỏ | Lớn hơn |
| Phụ thuộc lúc chạy | Cần đúng thư viện, đúng phiên bản trên máy đích | Tự chứa |
| Cập nhật lỗi bảo mật thư viện | Cập nhật một lần cho cả hệ thống | Phải build lại từng chương trình |
| Vấn đề tương thích | "Thiếu `libxxx.so.N`", phiên bản `glibc` cũ | Ít hơn nhiều |

### Kiểm tra phụ thuộc động

```bash
ldd ./myapp                 # Linux: liệt kê thư viện chia sẻ cần thiết
otool -L ./myapp            # macOS
objdump -p myapp.exe | grep "DLL Name"      # Windows (MinGW)
file ./myapp                # kiến trúc: "ELF 64-bit LSB pie executable, x86-64, dynamically linked"
```

Kết quả `ldd` ví dụ:

```text
linux-vdso.so.1
libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6
libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6
```

### Vấn đề tương thích `glibc` khi phát hành nhị phân Linux

Nhị phân build trên bản Linux **mới** dùng các ký hiệu `glibc` mới, nên **không chạy trên bản cũ hơn** (lỗi `version 'GLIBC_2.34' not found`). Cách xử lý:

1. **Build trên hệ cũ nhất bạn muốn hỗ trợ** (ví dụ container Ubuntu 20.04 hoặc CentOS/manylinux) — nhị phân đó chạy được trên bản mới hơn.
2. **Liên kết tĩnh hoàn toàn với `musl`** (mục 22.7) — không phụ thuộc `glibc`.
3. Phát hành bằng **container/AppImage/Flatpak** kèm sẵn thư viện.

### Liên kết tĩnh một phần và hoàn toàn

```bash
gcc -O2 main.c -o myapp -static                  # tĩnh hoàn toàn (với glibc có cảnh báo về getaddrinfo, dlopen, NSS)
gcc -O2 main.c -o myapp -static-libgcc           # chỉ libgcc
gcc -O2 main.c -o myapp -Wl,-Bstatic -lfoo -Wl,-Bdynamic   # tĩnh riêng libfoo
```

**Chú ý giấy phép:** liên kết tĩnh với thư viện **LGPL** có thể buộc bạn cung cấp cách người dùng thay thế thư viện đó; xem điều khoản.

## 22.6. Biên dịch chéo (cross-compilation)

Máy **build** (host) khác máy **chạy** (target): ví dụ build file `.exe` Windows trên Linux/macOS, hoặc file ARM trên máy x86.

### Khái niệm

Một **toolchain** đi kèm bộ tiền tố **triple**: `kiến_trúc-nhà_cung_cấp-hệ_điều_hành-abi`:

| Triple | Đích |
|---|---|
| `x86_64-w64-mingw32` | Windows 64-bit (MinGW-w64) |
| `i686-w64-mingw32` | Windows 32-bit |
| `aarch64-linux-gnu` | Linux ARM 64-bit (glibc) |
| `arm-linux-gnueabihf` | Linux ARM 32-bit (Raspberry Pi cũ) |
| `x86_64-linux-musl` | Linux x86-64 với musl (tĩnh) |
| `arm-none-eabi` | Vi điều khiển ARM (bare-metal) |

Trình biên dịch có tiền tố tương ứng: `x86_64-w64-mingw32-gcc`, `aarch64-linux-gnu-gcc`...

### Windows từ Linux/macOS: MinGW-w64

```bash
# Debian/Ubuntu
sudo apt install mingw-w64

x86_64-w64-mingw32-gcc -std=c11 -O2 -Wall src/*.c -Iinclude -o build/myapp.exe
x86_64-w64-mingw32-gcc ... -static -o build/myapp.exe        # tĩnh: không cần DLL của MinGW đi kèm
file build/myapp.exe        # PE32+ executable (console) x86-64, for MS Windows
wine build/myapp.exe        # (tùy chọn) thử nhanh bằng Wine
```

Không có `-static`, `.exe` có thể cần `libgcc_s_seh-1.dll`, `libwinpthread-1.dll` đi kèm; kiểm tra bằng `objdump -p`. Nhớ rằng mã dùng API POSIX (`fork`, `poll`, `pthread` trực tiếp) sẽ **không biên dịch được** cho Windows nếu không có lớp tương thích (chương 16, 15).

### Linux ARM từ x86

```bash
sudo apt install gcc-aarch64-linux-gnu
aarch64-linux-gnu-gcc -O2 src/*.c -Iinclude -o build/myapp-arm64
file build/myapp-arm64      # ELF 64-bit ... ARM aarch64
qemu-aarch64 -L /usr/aarch64-linux-gnu build/myapp-arm64      # thử bằng QEMU user-mode
```

### Cross-compile với Make và CMake

Make dùng biến `CC`: `make CC=x86_64-w64-mingw32-gcc`. Đừng để Makefile hard-code `gcc`, và tránh gọi chương trình vừa build (bước sinh mã) bằng compiler đích.

**CMake toolchain file** `mingw-w64.cmake`:

```cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)     # công cụ tìm trên máy host
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)      # thư viện tìm trong sysroot đích
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

```bash
cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=mingw-w64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-win
```

### Zig như một compiler chéo tiện lợi

`zig cc` là bản đóng gói `clang` kèm thư viện chuẩn của nhiều đích, cho phép cross-compile chỉ với **một công cụ**:

```bash
zig cc -target x86_64-windows-gnu  -O2 src/*.c -Iinclude -o build/myapp.exe
zig cc -target aarch64-linux-musl  -O2 src/*.c -Iinclude -o build/myapp-arm64
zig cc -target x86_64-linux-gnu.2.17 -O2 src/*.c -Iinclude -o build/myapp    # nhắm glibc 2.17 (chạy trên hệ cũ)
```

Tiện đặc biệt để nhắm **phiên bản `glibc` thấp** mà không cần container cũ.

## 22.7. Nhị phân Linux tĩnh với `musl`

**musl** là thư viện C chuẩn gọn, thiết kế cho liên kết tĩnh. Nhị phân tĩnh với musl **chạy trên hầu như mọi bản Linux** cùng kiến trúc, không lo phiên bản `glibc`.

```bash
sudo apt install musl-tools
musl-gcc -std=c11 -O2 -static src/*.c -Iinclude -o build/myapp-static
ldd build/myapp-static           # "not a dynamic executable"
file build/myapp-static          # "statically linked"
```

Trên Alpine Linux (mặc định dùng musl), build bằng `gcc` bình thường trong container Alpine: `docker run --rm -v "$PWD":/src -w /src alpine:3 sh -c "apk add build-base && make CFLAGS='-O2 -static'"`.

Lưu ý khi dùng musl: một số hành vi khác `glibc` (định dạng `printf` cho một số trường hợp, `getaddrinfo`/NSS đơn giản hơn, không hỗ trợ một số mở rộng GNU); hiệu năng `malloc` mặc định của musl thấp hơn nên có thể cần thay allocator. **Kiểm thử bản musl riêng**.

### Giảm kích thước và loại bỏ thông tin gỡ lỗi

```bash
gcc -Os -ffunction-sections -fdata-sections -Wl,--gc-sections main.c -o myapp     # bỏ hàm/dữ liệu không dùng
strip --strip-unneeded myapp                                                       # bỏ bảng ký hiệu
# hoặc giữ ký hiệu ở file riêng để debug sau:
objcopy --only-keep-debug myapp myapp.debug && strip myapp && objcopy --add-gnu-debuglink=myapp.debug myapp
```

Giữ file `.debug` của **mỗi bản phát hành** để đọc được stack trace khi người dùng báo crash.

## 22.8. Đóng gói bằng CPack, `.deb`, `.rpm`, Docker

### CPack (đi kèm CMake)

```cmake
set(CPACK_PACKAGE_NAME "myapp")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "ban@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Ung dung vi du bang C")
set(CPACK_GENERATOR "TGZ;ZIP;DEB")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)          # tự dò phụ thuộc thư viện động cho .deb
include(CPack)
```

```bash
cmake --build build && cd build && cpack          # sinh myapp-1.2.0-Linux.tar.gz, .zip, .deb
dpkg-deb --info myapp-1.2.0-Linux.deb
dpkg -c myapp-1.2.0-Linux.deb                      # liệt kê file trong gói
```

Với Windows, CPack có generator `NSIS`/`WIX` để tạo trình cài đặt `.exe`/`.msi`; với macOS: `DragNDrop` (`.dmg`).

### `fpm` — nhanh cho `.deb`/`.rpm` từ thư mục file

```bash
make DESTDIR=/tmp/pkgroot PREFIX=/usr install
fpm -s dir -t deb -n myapp -v 1.2.0 --description "Ung dung vi du" -C /tmp/pkgroot .
fpm -s dir -t rpm -n myapp -v 1.2.0 -C /tmp/pkgroot .
```

### Docker: multi-stage build cho ảnh nhỏ

Stage 1 dựng nhị phân với đầy đủ công cụ; stage 2 chỉ chứa nhị phân tĩnh — ảnh cuối chỉ vài MB, ít bề mặt tấn công.

```dockerfile
# ---- build ----
FROM alpine:3.20 AS build
RUN apk add --no-cache build-base
WORKDIR /src
COPY . .
RUN make CFLAGS="-O2 -static" && strip build/myapp

# ---- runtime ----
FROM scratch
COPY --from=build /src/build/myapp /myapp
USER 65534:65534                # chạy bằng người dùng không đặc quyền (nobody)
ENTRYPOINT ["/myapp"]
```

```bash
docker build -t myapp:1.2.0 .
docker run --rm myapp:1.2.0 --version
docker image ls myapp            # kiểm tra kích thước (thường vài trăm KB - vài MB)
```

`FROM scratch` là ảnh **rỗng hoàn toàn**; nhị phân phải **tĩnh** và không cần file hệ thống (chứng chỉ TLS, `/etc/passwd`...). Nếu cần, dùng `alpine` hoặc `gcr.io/distroless/static` để có sẵn chứng chỉ và tài khoản.

### Các kênh phân phối khác

| Kênh | Ghi chú |
|---|---|
| **GitHub Releases** | Đơn giản nhất: đính kèm tarball, zip, nhị phân, `SHA256SUMS` |
| **Homebrew (macOS/Linux)** | Viết *formula* trỏ tới tarball + SHA-256 |
| **AUR / gói của distro** | Người bảo trì viết `PKGBUILD`, spec... dựa trên gói nguồn của bạn |
| **vcpkg / Conan** | Trình quản lý phụ thuộc C/C++: thư viện của bạn có thể được dùng bằng một dòng cấu hình |
| **Snap / Flatpak / AppImage** | Đóng gói ứng dụng kèm phụ thuộc, chạy trên nhiều bản Linux |
| **Chocolatey / winget / Scoop** | Windows |

## 22.9. Ký số và chuỗi cung ứng

Người dùng tải nhị phân của bạn cần tin nó **thật sự do bạn tạo và chưa bị sửa**.

### Chữ ký với GPG hoặc minisign

```bash
gpg --armor --detach-sign myapp-1.2.0.tar.gz            # tạo myapp-1.2.0.tar.gz.asc
gpg --verify myapp-1.2.0.tar.gz.asc myapp-1.2.0.tar.gz  # người dùng kiểm tra bằng khóa công khai của bạn

minisign -Sm myapp-1.2.0.tar.gz                          # nhỏ gọn, hiện đại
```

Công bố **khóa công khai** ở nơi đáng tin (README, trang web, keyserver). Ký `SHA256SUMS` cũng đủ. Trên Windows/macOS còn có **ký mã (code signing)** để hệ điều hành không cảnh báo "nhà phát hành không xác định" (chứng chỉ trả phí; macOS cần *notarization*).

### Build có thể lặp lại (reproducible builds) và SBOM

- **Reproducible build:** hai lần build cùng mã nguồn cho **kết quả giống hệt từng byte** → người khác xác minh được bản nhị phân khớp mã nguồn. Cần loại bỏ yếu tố thay đổi: `__DATE__`/`__TIME__`, đường dẫn tuyệt đối (`-ffile-prefix-map=$PWD=.`), thứ tự file, múi giờ; đặt `SOURCE_DATE_EPOCH`.
- **SBOM (Software Bill of Materials):** danh sách phụ thuộc và phiên bản đi kèm bản phát hành, để người dùng biết bản của họ có chứa thư viện dính lỗ hổng nào (định dạng SPDX/CycloneDX).
- **Ghim phụ thuộc** (phiên bản/hash) và theo dõi CVE của thư viện bạn dùng (chương 18).

## 22.10. Tự động phát hành với GitHub Actions và Releases

Quy trình: bạn **gắn tag** `v1.2.0` và đẩy lên → CI **build cho nhiều nền tảng** → **tạo Release** đính kèm file.

```yaml
# .github/workflows/release.yml
name: Release

on:
  push:
    tags: ['v*']                       # kích hoạt khi đẩy tag dạng v1.2.0

permissions:
  contents: write                      # cần quyền ghi để tạo Release

jobs:
  build:
    name: Build ${{ matrix.target }}
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        include:
          - { os: ubuntu-latest, target: linux-x86_64,   cc: gcc,   ext: ""     }
          - { os: macos-latest,  target: macos-arm64,    cc: clang, ext: ""     }
          - { os: ubuntu-latest, target: windows-x86_64, cc: x86_64-w64-mingw32-gcc, ext: ".exe" }
    steps:
      - uses: actions/checkout@v4

      - name: Cai cong cu (Windows cheo)
        if: matrix.target == 'windows-x86_64'
        run: sudo apt-get update && sudo apt-get install -y mingw-w64

      - name: Build
        run: make CC=${{ matrix.cc }} CFLAGS="-O2 -static" || make CC=${{ matrix.cc }} CFLAGS="-O2"
        # macOS không hỗ trợ -static hoàn toàn nên có phương án dự phòng

      - name: Dong goi
        run: |
          VERSION=${GITHUB_REF_NAME#v}
          NAME=myapp-${VERSION}-${{ matrix.target }}
          mkdir -p dist/$NAME
          cp build/myapp${{ matrix.ext }} README.md LICENSE dist/$NAME/
          (cd dist && tar czf $NAME.tar.gz $NAME)
          (cd dist && (sha256sum $NAME.tar.gz 2>/dev/null || shasum -a 256 $NAME.tar.gz) > $NAME.tar.gz.sha256)

      - uses: actions/upload-artifact@v4
        with:
          name: ${{ matrix.target }}
          path: dist/*.tar.gz*

  release:
    needs: build
    runs-on: ubuntu-latest
    steps:
      - uses: actions/download-artifact@v4
        with:
          path: artifacts
          merge-multiple: true
      - name: Tao Release
        uses: softprops/action-gh-release@v2
        with:
          files: artifacts/*
          generate_release_notes: true
```

Cách phát hành:

```bash
# 1) cập nhật VERSION và CHANGELOG, commit
echo 1.2.0 > VERSION && git add -A && git commit -m "Release 1.2.0"
# 2) gắn tag có chú thích và đẩy
git tag -a v1.2.0 -m "myapp 1.2.0"
git push origin main --tags
```

GitHub tự chạy workflow; vài phút sau trang **Releases** có các file tải về. Người dùng tải file và kiểm tra `.sha256`.

**Kiểm tra sau khi phát hành:** tải chính file từ Release trên một máy sạch (hoặc container mới), kiểm tra checksum, chạy `--version` và một thao tác cơ bản.

## 22.11. Áp dụng cho chính cuốn sách này: phát hành sách

Kho mã của sách này (`manuscript/`, `formats/`, `kdp/`, `tools/`) cũng là một dự án cần "phát hành". Quy trình tương tự phần mềm:

1. **Nguồn duy nhất:** nội dung Markdown ở `manuscript/`.
2. **Sinh định dạng đầu ra tự động** (script/CI), không chỉnh tay file đầu ra:
   - **HTML** (trang web, GitHub Pages): `npm run build:html` → thư mục `dist/`.
   - **EPUB và PDF** cho bán/tải: script Python trong `tools/bookgen` (`python scripts/generate_book.py`) tạo `formats/book.epub`, `formats/book.pdf`.
3. **Kiểm thử đầu ra:** mở EPUB bằng trình đọc, kiểm tra mục lục, ảnh, ký tự tiếng Việt, khối mã (`epubcheck` kiểm định EPUB); duyệt PDF về ngắt trang, khối mã bị tràn lề.
4. **Kiểm thử mã trong sách:** biên dịch và chạy mọi ví dụ đầy đủ ở thư mục `code/` bằng CI (chương 21) — sách dạy lập trình mà mã không chạy được là lỗi nghiêm trọng.
5. **Đánh phiên bản và ghi thay đổi** cho từng ấn bản (1.0, 1.1...) và có mục errata.
6. **Đưa lên** GitHub Pages (đọc trực tiếp), và Releases (tải EPUB/PDF).

### Chuẩn bị hồ sơ Amazon KDP

Thư mục `kdp/` chứa các thứ cần cho **Kindle Direct Publishing**:

| Mục | Nội dung / lưu ý |
|---|---|
| **Bản thảo** | EPUB (hoặc DOCX/KPF) đã kiểm định; PDF cho bản in (kích thước trang, lề, số trang) |
| **Tiêu đề, phụ đề, tác giả** | Phải khớp giữa file bìa, trang tên sách và metadata |
| **Mô tả sách** | Đoạn giới thiệu hấp dẫn (`kdp/description.txt`), có thể dùng HTML cơ bản |
| **Từ khóa và danh mục** | Tối đa 7 từ khóa; chọn danh mục phù hợp (Computers & Technology → Programming) |
| **Bìa** | Ảnh bìa đúng kích thước/độ phân giải khuyến nghị (Kindle: cạnh dài ≥ 2.560 px, tỷ lệ ~1,6:1; bìa in cần bìa trọn gói gồm gáy và lề tràn) |
| **ISBN** | KDP cấp ISBN miễn phí (hoặc dùng ISBN riêng); ebook không bắt buộc |
| **Giá và quyền** | Chọn mức giá, lãnh thổ, chương trình tiền bản quyền (35% hay 70%) |
| **Giấy phép nội dung** | Ghi rõ bản quyền sách và giấy phép mã nguồn đi kèm |
| **Bản xem thử** | Dùng công cụ *Previewer* của KDP để xem thử trên các thiết bị |

Các file `kdp/checklist.md`, `kdp/metadata.yaml`, `kdp/cover-guidelines.md` tóm tắt danh sách kiểm tra. Nguyên tắc: **coi bản phát hành sách như bản phát hành phần mềm** — có checklist, có kiểm thử, có phiên bản, và có quy trình cập nhật khi phát hiện lỗi.

## 22.12. Danh sách kiểm tra phát hành

- [ ] `VERSION`, `CHANGELOG.md` đã cập nhật; SemVer đúng.
- [ ] Tất cả test đạt (ma trận nền tảng/compiler), sanitizer sạch, không cảnh báo.
- [ ] Bản `Release` (`-O2`, `-DNDEBUG`, cờ bảo mật chương 18), đã `strip` (giữ file `.debug`).
- [ ] Phụ thuộc kiểm tra (`ldd`); build trên hệ đủ cũ hoặc liên kết tĩnh/musl.
- [ ] `--version` in đúng phiên bản và commit.
- [ ] Gói nguồn **build được từ đầu** trong môi trường sạch.
- [ ] `LICENSE`, `README`, tài liệu (man page) có trong gói; giấy phép phụ thuộc tương thích.
- [ ] Checksum (`SHA256SUMS`) và chữ ký; khóa công khai được công bố.
- [ ] Tag Git tương ứng (`vX.Y.Z`); Release được tạo với ghi chú thay đổi.
- [ ] Thử tải và chạy bản phát hành trên máy sạch.
- [ ] Có kênh nhận báo lỗi (issue tracker) và chính sách bảo mật (`SECURITY.md`).

## 22.13. Lỗi thường gặp

| Lỗi | Hậu quả | Cách tránh |
|---|---|---|
| Hard-code đường dẫn cài đặt | Không đóng gói được | `PREFIX`/`DESTDIR`; đường dẫn tương đối tới tài nguyên |
| Gói nguồn thiếu file | Người dùng không build được | Build thử từ tarball sạch |
| Build trên hệ quá mới | `GLIBC_x.y not found` trên máy người dùng | Build trên hệ cũ, hoặc musl/`zig cc` |
| Quên `-static`/DLL đi kèm (Windows) | "Không tìm thấy libgcc_s_seh-1.dll" | `-static` hoặc đóng gói DLL |
| Không `strip`/giữ debug symbols | File quá lớn / không debug được | `strip` + lưu `.debug` riêng |
| Phát hành `-O0`, có assert, có sanitizer | Chậm, lộ thông tin | Build `Release`, `-DNDEBUG` |
| Không có checksum/chữ ký | Không xác minh được | `SHA256SUMS` + chữ ký |
| Hard-code compiler `gcc` | Không cross-compile được | Dùng biến `CC` |
| Không ghi phiên bản trong chương trình | Khó tái hiện lỗi | `--version` nhúng lúc build |
| Bỏ qua giấy phép của thư viện | Rủi ro pháp lý | Rà soát và ghi `THIRD_PARTY_NOTICES` |
| Phát hành thủ công dễ sai | Sót bước | Tự động hóa bằng CI |

## 22.14. Tóm tắt

- Phát hành nghiêm túc cần **phiên bản (SemVer)**, **giấy phép**, **changelog**, và **quy trình lặp lại được**.
- Makefile/CMake nên hỗ trợ `PREFIX`, `DESTDIR`, `install`, `uninstall`, `dist`; tôn trọng `CC`/`CFLAGS`/`LDFLAGS`.
- Gói nguồn tạo từ `git archive`; **kiểm tra build từ tarball sạch**; kèm `SHA256SUMS` và chữ ký.
- Liên kết động gọn nhưng phụ thuộc hệ thống; tĩnh/musl cho nhị phân Linux tự chứa; chú ý tương thích `glibc`.
- **Cross-compile** bằng `mingw-w64`, toolchain `*-linux-gnu`, `zig cc`; dùng CMake toolchain file.
- Docker multi-stage tạo ảnh nhỏ; CPack/`fpm` tạo `.deb`/`.rpm`; nhiều kênh phân phối khác.
- **GitHub Actions** tự build đa nền tảng và tạo **Release** khi gắn tag.
- Sách này cũng được phát hành theo quy trình như phần mềm: sinh EPUB/PDF tự động, kiểm thử mã, checklist KDP.

## 22.15. Bài tập

1. Thêm `install`, `uninstall`, `dist` và `--version` vào một dự án của bạn. Kiểm tra `make DESTDIR=/tmp/pkgroot PREFIX=/usr install` tạo cây thư mục đúng bằng `find /tmp/pkgroot -type f`.
2. Tạo `myapp-X.Y.Z.tar.gz` bằng `git archive`, giải nén vào thư mục trống, `make && make test`. Cố ý xóa một file nguồn khỏi git rồi quan sát lỗi để hiểu vì sao bước kiểm tra này cần thiết.
3. Biên dịch chương trình ví dụ thành: (a) động, (b) tĩnh với `-static`, (c) tĩnh với `musl-gcc`. So sánh kích thước, kết quả `ldd`/`file`, và thử chạy (b), (c) trong container `alpine` và `debian` rỗng.
4. Cross-compile cho Windows bằng `mingw-w64`, kiểm tra bằng `file` và `objdump -p`; chạy thử bằng Wine hoặc trên máy Windows. Ghi lại những đoạn mã phải sửa cho di động.
5. Viết CMake toolchain file cho `aarch64-linux-gnu`, build và chạy thử bằng `qemu-aarch64`.
6. Viết `Dockerfile` multi-stage cho dự án của bạn; đo kích thước ảnh với `FROM scratch` so với `FROM debian`.
7. Tạo cặp khóa GPG (hoặc `minisign`), ký `SHA256SUMS`, và viết hướng dẫn 3 dòng cho người dùng xác minh.
8. Viết `.github/workflows/release.yml` để khi đẩy tag `v*` sẽ build Linux/macOS/Windows, tạo tarball + checksum và đính kèm vào Release. Thử với tag `v0.0.1-test` rồi xóa.
9. Tạo build **có thể lặp lại**: build hai lần ở hai thư mục khác nhau và so `sha256sum`; nếu khác, tìm nguyên nhân (đường dẫn, timestamp) và khắc phục bằng `-ffile-prefix-map` và `SOURCE_DATE_EPOCH`.
10. (Thử thách cho sách) Viết một script `make book` chạy toàn bộ: biên dịch mọi ví dụ trong `code/` (báo lỗi nếu không biên dịch được), build HTML, EPUB, PDF, chạy `epubcheck`, và tạo `SHA256SUMS`; tích hợp vào CI để mỗi lần đẩy mã đều kiểm tra sách.

Mã nguồn mẫu: /code/chapter-22
