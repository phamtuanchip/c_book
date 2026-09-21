Chapter 13 code samples

Files:
- error_wrapper.c: example wrapper for fopen with errno logging

Build:
- gcc -Wall -Wextra -std=c11 -o error_wrapper error_wrapper.c

Run:
- ./error_wrapper somefile.txt
