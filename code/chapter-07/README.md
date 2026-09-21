Chapter 7 code samples

Files:
- reverse_string.c: reverse input string (uses fgets)
- dynamic_string.c: builds a dynamic string by reading stdin and reallocating
- Makefile: build targets

Build:
- make

Or:
- gcc -Wall -Wextra -std=c11 -o reverse_string reverse_string.c
- gcc -Wall -Wextra -std=c11 -o dynamic_string dynamic_string.c

Run:
- ./reverse_string
- ./dynamic_string  # read until EOF
