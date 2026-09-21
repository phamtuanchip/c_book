Chapter 18 code samples - security & memory safety

Files:
- buffer_overflow.c: intentionally unsafe example (educational only)
- overflow_fixed.c: safe replacement using fgets and bounds checks

Build with ASAN to detect issues:
- gcc -fsanitize=address -g -o overflow buffer_overflow.c
- gcc -Wall -Wextra -std=c11 -o overflow_fixed overflow_fixed.c

Notes: explain why gets() is unsafe and removed from standards; prefer fgets/strncpy with care.
