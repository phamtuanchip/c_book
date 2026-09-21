Chapter 20 code samples - simple web server

Files:
- simple_http_server.c: minimal HTTP server that returns a fixed response

Build:
- gcc -Wall -Wextra -std=c11 -o http_server simple_http_server.c

Run:
- ./http_server 8080
- curl http://127.0.0.1:8080/

Notes: not suitable for production. Next steps: add routing, static file serving, thread pool.
