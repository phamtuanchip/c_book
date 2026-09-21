# formats/

Thư mục chứa file xuất bản và tài nguyên build.

| File/Thư mục | Mô tả |
|---|---|
| `book.epub` | EPUB3 được sinh ra bởi build script |
| `book.pdf` | PDF được sinh ra bởi build script |
| `cover.png` | Ảnh bìa (sinh bằng Pillow) |
| `styles.css` | CSS cho EPUB |
| `fonts/` | Font TTF nhúng vào EPUB (ví dụ: NotoSans) |
| `code_samples.zip` | ZIP code mẫu toàn bộ /code |
| `kdp_upload_package.zip` | Gói upload KDP (epub + cover + metadata) |

Build bằng Python từ thư mục gốc repo:

```bash
python scripts/generate_book.py epub   # sinh EPUB
python scripts/generate_book.py pdf    # sinh PDF
```

Xem hướng dẫn đầy đủ trong README.md ở thư mục gốc.
