Chapter 9 code samples

Files:
- leak_demo.c: demonstrates memory leak by not freeing allocations
- use_after_free.c: returns pointer to freed memory (use-after-free)
- double_free.c: example of double free
- Makefile: build targets

Build:
- make

Notes:
- Run under Valgrind (Linux) or use AddressSanitizer to detect issues:
  valgrind ./leak_demo
  gcc -fsanitize=address -g -o use_after_free use_after_free.c
