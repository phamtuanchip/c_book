# Chương 12 — File I/O & thao tác hệ thống

Mục tiêu chương:

- Đọc/ghi file text và binary bằng fopen/fgets/fscanf/fprintf/fread/fwrite.
- Hiểu buffering, modes, và xử lý lỗi I/O; khái niệm mmap và portability.

1. File text I/O

- fopen(path, "r"/"w"/"a") để mở file.
- fgets để đọc line an toàn; fprintf để ghi.
- Kiểm tra return của fopen, fgets, fprintf; dùng perror hoặc strerror(errno).

2. Binary I/O

- fread/fwrite để đọc/ghi block bytes; hữu ích cho file nhị phân hoặc sao chép file.
- Ví dụ copy: đọc buffer 8KB và fwrite cho tới hết file.

3. Buffering và fflush

- stdout thường được buffered line-by-line hoặc fully buffered; fflush để đảm bảo ghi ra disk.
- File modes: "rb"/"wb" cho binary trên Windows

4. mmap (khái niệm)

- mmap ánh xạ file vào bộ nhớ — nhanh cho đọc/ghi lớn; portability khác nhau giữa POSIX và Windows (MapViewOfFile).
- Giải thích ngắn, khuyến nghị dùng fread/fwrite cho portability trừ khi cần hiệu năng đặc biệt.

5. Ví dụ thực hành

- /code/chapter-12/copy_file.c: copy binary file with fread/fwrite.
- /code/chapter-12/csv_parser.c: simple CSV reader handling quoted fields.

Bài tập

- Viết chương trình merge nhiều file text vào một file, xử lý lỗi I/O.
- Viết chương trình tìm kiếm chuỗi trong file lớn bằng buffer streaming.

Ghi chú: kèm README chỉ dẫn build/run và test files.
