# Học lập trình C — Từ cơ bản đến nâng cao
**Đọc online (GitHub Pages): https://phamtuanchip.github.io/c_book/**

## Mục tiêu

Bộ sách dạy lập trình C đầy đủ, từ cơ bản đến nâng cao, hướng tới xuất bản trên Amazon KDP.

## Độc giả mục tiêu

- Người mới bắt đầu (chưa biết lập trình)
- Sinh viên khoa CNTT/Điện tử muốn học sâu về C
- Lập trình viên muốn ôn lại nền tảng hoặc chuyển sang ngôn ngữ hệ thống

## Nội dung sách (22 chương)

1. Lời tựa và hướng dẫn sử dụng sách
2. Khái niệm cơ bản về lập trình máy tính: dữ liệu, biến, kiểu dữ liệu, luồng điều khiển, hàm
3. Ngôn ngữ máy và lịch sử C: từ máy tới Assembly, lịch sử C và định hướng thiết kế
4. Môi trường phát triển: GCC/MinGW/MSYS2 trên Windows, GCC trên Linux, clang trên macOS
5. Cú pháp cơ bản và cấu trúc chương trình C
6. Con trỏ, quản lý bộ nhớ, struct, liên kết danh sách, file I/O
7. Lập trình có cấu trúc và mô-đun: header, makefile, tổ chức dự án
8. Kỹ thuật nâng cao: macro, tiền xử lý, multi-threading (POSIX), socket cơ bản
9. Debugging và testing: gdb, valgrind, unit test
10. Best practices: an toàn bộ nhớ, phong cách mã, tối ưu hóa
11–22. Ứng dụng thực tế, mini project, phụ lục

## Cấu trúc repository

```
/manuscript   — nguồn sách (Markdown, chapter-01.md … chapter-22.md)
/code         — mã nguồn ví dụ theo chương (với Makefile và README)
/assets       — hình ảnh, sơ đồ, biểu đồ
/formats      — file xuất (book.epub, book.pdf, styles.css, fonts/)
/kdp          — metadata, checklist, hướng dẫn Amazon KDP
/tools/bookgen — Python package sinh sách (epub/pdf/cover/package)
/scripts      — entry point wrapper: generate_book.py
```

## Sinh sách EPUB và PDF (Python)

Toàn bộ quy trình build được thực hiện bằng Python. Không cần shell script hay PowerShell.

### Yêu cầu

| Công cụ | Mục đích | Cài đặt |
|---------|----------|---------|
| Python 3.8+ | chạy build script | python.org |
| pandoc | convert Markdown → EPUB/PDF | pandoc.org |
| TeX engine (lualatex/xelatex) | tạo PDF | MiKTeX / TeX Live |
| Pillow (tùy chọn) | sinh ảnh bìa | `pip install pillow` |

```bash
pip install -r requirements.txt   # cài Pillow (tùy chọn)
```

### Lệnh build

Chạy từ thư mục gốc repo:

```bash
# Sinh EPUB
python scripts/generate_book.py epub

# Sinh PDF
python scripts/generate_book.py pdf

# Sinh ảnh bìa (cần Pillow)
python scripts/generate_book.py cover

# Đóng gói code mẫu thành ZIP
python scripts/generate_book.py codezip

# Đóng gói upload KDP (epub + cover + metadata)
python scripts/generate_book.py package
```

File đầu ra nằm trong `/formats/`:
- `formats/book.epub`
- `formats/book.pdf`
- `formats/cover.png`
- `formats/code_samples.zip`
- `formats/kdp_upload_package.zip`

### Tùy chỉnh

```bash
# Chỉ định đường dẫn output
python scripts/generate_book.py epub --output my_book.epub

# Chỉ định repo root khác
python scripts/generate_book.py pdf --repo /path/to/repo
```

### Cách hoạt động

`tools/bookgen/generator.py` (class `BookGenerator`):
- Thu thập các file `manuscript/chapter-*.md` theo thứ tự
- Tiền xử lý markdown: normalize unicode, tự động fence code C, xử lý ký tự đặc biệt
- Gọi `pandoc` với tham số phù hợp để sinh EPUB3 hoặc PDF (qua lualatex)
- Nhúng font từ `formats/fonts/*.ttf` vào EPUB nếu có
- Đọc metadata từ `kdp/metadata.yaml`

## Cài đặt môi trường lập trình C

- **Windows**: cài MSYS2 — `pacman -S mingw-w64-x86_64-gcc make gdb`
- **Linux**: `sudo apt install build-essential gdb valgrind`
- **macOS**: `brew install gcc gdb`
- **Editor**: VSCode (extension C/C++), CLion, Vim/Emacs

## Chuẩn xuất bản Amazon KDP

- Format: EPUB3 (upload trực tiếp) hoặc KPF (qua Kindle Create)
- Quy trình: build EPUB → kiểm tra bằng Kindle Previewer → upload lên KDP
- Metadata, từ khóa, bìa: xem `/kdp/`

## Đóng góp

Gửi PR hoặc issue qua GitHub cho bất kỳ phần nào: nội dung, sửa lỗi, code mẫu.

## Bản quyền

- Code: MIT License
- Nội dung sách: Creative Commons CC BY-NC

## Liên hệ

Tác giả: Lukas — phamtuanchip@gmail.com
