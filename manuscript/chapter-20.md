# Chương 20 — Project: Web server đơn giản

Mục tiêu chương:

- Xây một web server nhỏ có thể serve static files và vài route động (JSON API).
- Hiểu trade-offs giữa thread-per-connection, thread-pool và non-blocking event loop.

1. Architecture cơ bản

- Accept loop: accept() kết nối và xử lý request.
- Thread-per-connection: đơn giản nhưng tốn tài nguyên khi nhiều client.
- Thread pool: queue kết nối và worker threads xử lý; cân bằng throughput.

2. HTTP request parsing

- Parsers đơn giản: đọc request line, headers, method, path.
- Không cần implement full HTTP/1.1 — focus vào GET và simple headers.

3. Serving static files

- Map path -> filesystem path; validate path để tránh directory traversal.
- Set Content-Type theo file extension (MIME map).
- Use sendfile on POSIX for efficiency where có.

4. Routing and logging

- Map routes to handler functions; basic router supports prefix/static routes.
- Logging: request time, method, path, response code.

5. Ví dụ thực hành

- /code/chapter-20/simple_server.c: thread-per-connection server serving files from ./www
- /code/chapter-20/thread_pool_server.c: server using fixed worker pool and request queue

Bài tập

- Implement server serving static files and a /api/time JSON endpoint.
- Add graceful shutdown (signal handling) and basic rate-limiting.

Ghi chú: provide POSIX build instructions; Windows users see alternative using Winsock or run in WSL.