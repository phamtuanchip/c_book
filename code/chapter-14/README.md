Chapter 14 code samples

Files:
- macro_log.c: demonstrates LOG macro using __FILE__ and __LINE__

Build:
- gcc -Wall -Wextra -std=c11 -o macro_log macro_log.c

Run:
- ./macro_log

Notes:
- LOG implemented as do { ... } while(0) to be statement-safe.
