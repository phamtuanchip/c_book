Chapter 15 code samples

Files:
- producer_consumer.c: example using pthreads, mutex, condition variables

Build (Linux):
- gcc -Wall -Wextra -std=c11 -pthread -o prodcons producer_consumer.c

Run:
- ./prodcons

Notes:
- pthreads are POSIX; on Windows use pthreads-win32 or adapt to Win32 threads.
