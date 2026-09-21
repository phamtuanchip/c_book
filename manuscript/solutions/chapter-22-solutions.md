# Chương 22 — Lời giải bài tập

Phần lớn bài tập chương này là **quy trình dòng lệnh**; lời giải dưới dạng lệnh và kết quả cần quan sát.

## Bài 1: `install`, `uninstall`, `dist`, `--version`

Dùng khối Makefile ở mục 22.3. Kiểm tra cây thư mục cài đặt:

```bash
make DESTDIR=/tmp/pkgroot PREFIX=/usr install
find /tmp/pkgroot -type f | sort
# /tmp/pkgroot/usr/bin/myapp
# /tmp/pkgroot/usr/share/doc/myapp/LICENSE
# /tmp/pkgroot/usr/share/doc/myapp/README.md
# /tmp/pkgroot/usr/share/man/man1/myapp.1
```

`--version` (mục 22.2) in `myapp 1.2.0`. Đường dẫn cài đặt phải nằm dưới `$(DESTDIR)$(PREFIX)`, **không** dưới thư mục hiện tại.

## Bài 2: gói nguồn bằng `git archive`, kiểm tra từ tarball sạch

```bash
git archive --format=tar.gz --prefix=myapp-1.2.0/ -o dist/myapp-1.2.0.tar.gz HEAD
rm -rf /tmp/verify && mkdir /tmp/verify && tar xzf dist/myapp-1.2.0.tar.gz -C /tmp/verify
(cd /tmp/verify/myapp-1.2.0 && make && make test)
```

Nếu bạn `git rm` một file nguồn (nhưng vẫn giữ nó ở thư mục làm việc rồi `git commit`), `make` trong `/tmp/verify` sẽ lỗi thiếu file — đó là lỗi mà người dùng thật sẽ gặp và bước kiểm tra này chặn được. Lưu ý `git archive` chỉ lấy file **đã commit**; file chưa `git add` cũng bị thiếu.

## Bài 3: động, tĩnh, `musl`

```bash
gcc -O2 main.c -o app_dyn
gcc -O2 -static main.c -o app_static
musl-gcc -O2 -static main.c -o app_musl
ls -l app_*                    # dyn ~16 KB; static (glibc) ~800 KB; musl ~20-100 KB
ldd app_dyn                    # liệt kê libc.so.6...
ldd app_static                 # "not a dynamic executable"
file app_musl                  # "statically linked"
docker run --rm -v "$PWD":/a alpine /a/app_musl        # chạy trong Alpine: OK (không cần glibc)
docker run --rm -v "$PWD":/a debian:stable-slim /a/app_static
```

Nhị phân tĩnh (b, c) chạy được trong container rỗng; bản động (a) chạy được nếu container có cùng `glibc` tương thích, và **không** chạy trong Alpine (thiếu `glibc`).

## Bài 4: cross-compile Windows

```bash
x86_64-w64-mingw32-gcc -std=c11 -O2 -static src/*.c -Iinclude -o build/myapp.exe
file build/myapp.exe                   # PE32+ executable (console) x86-64, for MS Windows
objdump -p build/myapp.exe | grep "DLL Name"     # chỉ KERNEL32.dll, msvcrt.dll... (không cần DLL của MinGW nhờ -static)
wine build/myapp.exe --version
```

Những chỗ thường phải sửa để di động: `fork`/`poll`/`pthread` (POSIX) → Winsock/Win32 hoặc lớp tương thích; dấu phân cách đường dẫn; `\n` ↔ `\r\n` với file văn bản mở `"r"`/`"w"` (dùng `"rb"`/`"wb"` cho nhị phân); `%zu` với runtime cũ của MSVC; `long` chỉ 32 bit.

## Bài 5: toolchain file cho ARM64

```cmake
# aarch64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

```bash
cmake -S . -B build-arm -DCMAKE_TOOLCHAIN_FILE=aarch64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-arm
qemu-aarch64 -L /usr/aarch64-linux-gnu build-arm/myapp --version
```

## Bài 6: Docker multi-stage

Dùng `Dockerfile` ở mục 22.8. So sánh: `FROM scratch` với nhị phân tĩnh ~1 MB cho ảnh ~1 MB; `FROM debian:stable-slim` + nhị phân động cho ~80 MB. Ảnh `scratch` không có shell (`docker exec ... sh` không chạy được) và không có chứng chỉ TLS/`/etc/passwd`; nếu ứng dụng cần, dùng `gcr.io/distroless/static` hoặc `alpine`.

## Bài 7: ký `SHA256SUMS`

```bash
gpg --armor --detach-sign SHA256SUMS                     # tạo SHA256SUMS.asc
# hoặc: minisign -Sm SHA256SUMS
```

Hướng dẫn người dùng (3 dòng):

```bash
gpg --import public-key.asc                              # nhập khóa công khai của tác giả (kiểm tra vân tay!)
gpg --verify SHA256SUMS.asc SHA256SUMS                   # chữ ký hợp lệ?
sha256sum -c SHA256SUMS --ignore-missing                 # file tải về khớp checksum?
```

## Bài 8: `release.yml`

Dùng workflow ở mục 22.10. Thử với tag thử: `git tag v0.0.1-test && git push origin v0.0.1-test`; kiểm tra tab **Actions** và **Releases**; dọn dẹp: `gh release delete v0.0.1-test --yes && git push --delete origin v0.0.1-test && git tag -d v0.0.1-test`. Nếu workflow không chạy: kiểm tra `on.push.tags`, quyền `contents: write`, và tag đã được đẩy lên.

## Bài 9: build có thể lặp lại

```bash
export SOURCE_DATE_EPOCH=$(git log -1 --format=%ct)
CFLAGS="-O2 -ffile-prefix-map=$PWD=." make -C /tmp/a clean all
CFLAGS="-O2 -ffile-prefix-map=$PWD=." make -C /tmp/b clean all
sha256sum /tmp/a/build/myapp /tmp/b/build/myapp          # hai giá trị PHẢI giống nhau
```

Nguyên nhân thường gặp khi khác nhau: đường dẫn tuyệt đối nhúng trong thông tin gỡ lỗi (`-ffile-prefix-map`/`-fdebug-prefix-map`), `__DATE__`/`__TIME__` trong mã (bỏ hoặc thay bằng `SOURCE_DATE_EPOCH`), thứ tự file từ `$(wildcard ...)` (dùng `$(sort ...)`), và bản đồ liên kết chứa đường dẫn. Kiểm tra khác biệt bằng `diffoscope a b`.

## Bài 10: `make book`

```make
BOOK_DIR := .

.PHONY: book check-code html epub pdf
book: check-code html epub pdf checksums

check-code:
	node tools/extract-code.js
	node tools/check-code.js               # biên dịch mọi ví dụ; thất bại nếu có lỗi

html:
	npm run build:html

epub pdf:
	python scripts/generate_book.py

checksums:
	cd formats && sha256sum book.epub book.pdf > SHA256SUMS
```

Trong CI: một job chạy `make book`, chạy `epubcheck formats/book.epub` (`java -jar epubcheck.jar`), rồi tải EPUB/PDF lên Release khi gắn tag. Nếu `check-code` thất bại, sách **không** được phát hành — đảm bảo mọi đoạn mã trong sách biên dịch được.
