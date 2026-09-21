# Chương 16 — Mạng cơ bản (sockets)

Mục tiêu chương:

- Hiểu nguyên tắc hoạt động của TCP/UDP sockets và viết client-server đơn giản.
- Biết xử lý blocking vs non-blocking sockets và cơ chế select/poll.

1. Cơ bản về socket API

- socket(), bind(), listen(), accept(), connect(), close().
- sockaddr_in, htons/ntohs, inet_pton/inet_ntop để chuyển đổi địa chỉ.

2. Giao tiếp: send/recv vs read/write

- send/recv cung cấp flags; read/write là wrapper POSIX.
- Luôn kiểm tra giá trị trả về — có thể partial send/recv.

3. Blocking vs non-blocking và multiplexing

- Blocking socket chặn khi không có dữ liệu; non-blocking trả -1 với EAGAIN/EWOULDBLOCK.
- select()/poll()/epoll để multiplex nhiều socket trong một thread.

4. Ví dụ thực hành

- echo_server.c: server TCP đơn giản multi-threaded (accept -> spawn worker thread)
- http_request.c: gửi HTTP GET thô tới server và in response headers

Bài tập

- Viết server echo TCP xử lý nhiều client (thread-per-connection) và phiên bản dùng select để xử lý nhiều client trong một thread.

Ghi chú: kèm hướng dẫn build cho POSIX; trên Windows dùng Winsock (khác tên hàm: WSAStartup, closesocket).