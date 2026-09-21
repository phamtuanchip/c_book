Chapter 16 code samples - networking (sockets)

Files:
- tcp_server.c: simple blocking TCP echo server (IPv4)
- tcp_client.c: simple TCP client to send lines and receive echo

Build (Linux/macOS):
- gcc -Wall -Wextra -std=c11 -o tcp_server tcp_server.c
- gcc -Wall -Wextra -std=c11 -o tcp_client tcp_client.c

Run server:
- ./tcp_server 8080

Run client:
- ./tcp_client 127.0.0.1 8080

Notes: sockets are POSIX; on Windows adapt or use Winsock (requires different setup).
