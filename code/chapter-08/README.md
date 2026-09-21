Chapter 8 code samples

Files:
- pointer_examples.c: basic pointer usage and pointer arithmetic
- qsort_cmp.c: comparator function used with qsort
- Makefile: build targets

Build:
- make

Or:
- gcc -Wall -Wextra -std=c11 -o pointer_examples pointer_examples.c
- gcc -Wall -Wextra -std=c11 -o qsort_cmp qsort_cmp.c

Run:
- ./pointer_examples
- ./qsort_cmp

Notes:
- Study cmp_int signature: it matches qsort comparator type.
