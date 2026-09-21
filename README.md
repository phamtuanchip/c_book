# Học lập trình C — Từ cơ bản đến nâng cao

**Đọc online (GitHub Pages): https://phamtuanchip.github.io/c_book/**

Giáo trình lập trình C bằng tiếng Việt: từ chương trình đầu tiên đến con trỏ, quản lý bộ nhớ, đa luồng, socket, bảo mật, kiểm thử, và hai dự án hoàn chỉnh (trình thông dịch mini, web server). Mỗi chương giải thích khái niệm từng bước, có sơ đồ bộ nhớ, bảng lỗi thường gặp, ví dụ chạy được và bài tập có lời giải.

## Độc giả mục tiêu

- Người mới bắt đầu (chưa biết lập trình).
- Sinh viên CNTT / Điện tử muốn học sâu về C và hệ thống.
- Lập trình viên muốn ôn nền tảng hoặc chuyển sang ngôn ngữ hệ thống.

## Nội dung (22 chương + lời giải)

| Phần | Chương |
|---|---|
| **I. Nhập môn** | 1. Giới thiệu và chương trình C đầu tiên · 2. Máy tính và lập trình cơ bản · 3. Lịch sử và triết lý của C · 4. Cú pháp & cấu trúc chương trình |
| **II. Nền tảng ngôn ngữ** | 5. Điều khiển luồng · 6. Hàm & phạm vi biến · 7. Mảng & chuỗi · 8. Con trỏ chi tiết |
| **III. Bộ nhớ, dữ liệu, xây dựng** | 9. Quản lý bộ nhớ và lỗi phổ biến · 10. Struct, union, enum · 11. Header, Makefile, build systems · 12. File I/O & thao tác hệ thống · 13. Xử lý lỗi |
| **IV. Kỹ thuật nâng cao** | 14. Tiền xử lý & macro · 15. Đa luồng & đồng bộ · 16. Mạng cơ bản (sockets) · 17. Tối ưu hóa & profiling · 18. Bảo mật & an toàn bộ nhớ |
| **V. Dự án và phát hành** | 19. Trình biên dịch / interpreter mini · 20. Web server đơn giản · 21. Testing & CI · 22. Packaging & deploy |
| **Phụ lục** | Lời giải bài tập của cả 22 chương |

## Cấu trúc repository

```text
manuscript/            nguồn sách (Markdown): chapter-01.md … chapter-22.md
manuscript/solutions/  lời giải: chapter-NN-solutions.md
code/chapter-NN/       ví dụ của từng chương (trích tự động từ bản thảo, kèm Makefile + README)
code/solutions/        mã trong phần lời giải
code/manifest.json     danh sách chương trình để kiểm tra biên dịch
tools/                 build-html.js (trang web), extract-code.js, check-code.js, bookgen/ (EPUB/PDF)
dist/                  trang HTML đã build (xuất bản lên GitHub Pages)
formats/               EPUB, PDF, bìa, font
kdp/                   metadata, checklist, hướng dẫn Amazon KDP
scripts/               generate_book.py (điểm vào để sinh EPUB/PDF)
.github/workflows/     GitHub Actions: deploy Pages, kiểm tra mã trong sách
```

## Quy trình làm việc

Bản thảo trong `manuscript/` là **nguồn duy nhất**. Mã trong `code/` được sinh từ đó để sách và mã luôn khớp nhau.

```bash
npm install                     # lần đầu: cài markdown-it, highlight.js
npm run build:html              # bản thảo → dist/ (trang web kiểu sách)
npm run extract:code            # bản thảo → code/ (trích ví dụ, Makefile, README)
npm run check:code              # biên dịch mọi ví dụ (mặc định CC=gcc)
```

- Ví dụ đầy đủ trong sách là khối ` ```c ` có **dòng đầu** dạng `// ten_file.c`; script trích ra file cùng tên. Khối có ghi chú `(phần chính…)`/`(ý tưởng)` bị bỏ qua. `calc.c` (chương 19) và `server.c` (chương 20) được ghép từ nhiều khối.
- **Đừng sửa trực tiếp trong `code/`** — sửa trong `manuscript/` rồi chạy `npm run extract:code`.
- Kiểm tra bằng compiler khác hoặc biên dịch chéo: `CC="clang" node tools/check-code.js`, `CC="zig cc -target x86_64-linux-musl" node tools/check-code.js`. Thêm `WARN=1` để coi cảnh báo là lỗi.
- Trang web tự cập nhật khi push lên `main` (workflow `pages.yml`).

### Trang web

Giao diện kiểu sách: chữ có chân, cột đọc hẹp, mục lục theo phần, mục lục trong chương, bảng kiểu sách in, ba chế độ nền **Sáng / Sepia / Tối** (nhớ lựa chọn), chỉnh cỡ chữ, nút sao chép mã, điều hướng chương trước/sau và stylesheet cho in ấn (Ctrl+P).

## Sinh sách EPUB và PDF (Python)

Yêu cầu: Python 3.8+, [pandoc](https://pandoc.org), một TeX engine (lualatex/xelatex) để tạo PDF, Pillow (tùy chọn, để sinh ảnh bìa).

```bash
pip install -r requirements.txt

python scripts/generate_book.py epub      # formats/book.epub
python scripts/generate_book.py pdf       # formats/book.pdf
python scripts/generate_book.py cover     # formats/cover.png (cần Pillow)
python scripts/generate_book.py codezip   # formats/code_samples.zip
python scripts/generate_book.py package   # formats/kdp_upload_package.zip
```

`tools/bookgen/generator.py` gom `manuscript/chapter-*.md` theo thứ tự, tiền xử lý Markdown, gọi pandoc để tạo EPUB3/PDF và đọc metadata từ `kdp/metadata.yaml`.

## Môi trường lập trình C

- **Windows:** MSYS2 (`pacman -S mingw-w64-ucrt-x86_64-toolchain make gdb`) hoặc **WSL** (khuyến nghị cho chương 15–16, 20 vì dùng POSIX).
- **Linux:** `sudo apt install build-essential gdb valgrind`.
- **macOS:** `xcode-select --install` (clang) và `brew install make`.
- Trình soạn thảo: VS Code (extension C/C++), CLion, Vim/Emacs.

Biên dịch mọi ví dụ trong sách với `-std=c11 -Wall -Wextra`.

## Xuất bản Amazon KDP

Định dạng EPUB3 (tải trực tiếp) hoặc KPF (qua Kindle Create). Quy trình: build EPUB → kiểm tra bằng Kindle Previewer / `epubcheck` → tải lên KDP. Metadata, từ khóa, bìa và danh sách kiểm tra nằm trong `kdp/` (xem thêm mục 22.11 của sách).

## Đóng góp

Gửi issue hoặc PR cho nội dung, lỗi chính tả, hoặc mã ví dụ. Khi sửa mã, sửa trong bản thảo, chạy `npm run extract:code && npm run check:code` và kèm kết quả.

## Bản quyền

- Mã nguồn: MIT License.
- Nội dung sách: Creative Commons CC BY-NC.

## Liên hệ

Tác giả: Lukas — phamtuanchip@gmail.com
