Chapter 17 code samples - profiling

Files:
- profile_demo.c: CPU-bound recursive Fibonacci to create hotspots

Profile examples:
- gprof: compile with -pg then run and gprof ./a.out gmon.out
- perf: perf record ./a.out; perf report
- valgrind + callgrind: valgrind --tool=callgrind ./a.out; kcachegrind callgrind.out.*

Notes: use -g to keep symbol info.
