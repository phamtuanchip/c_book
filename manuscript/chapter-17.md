# Chương 17 — Tối ưu hóa & profiling

Mục tiêu chương:

- Học cách đo hiệu năng thực tế, xác định hotspots và tối ưu hóa phù hợp.
- Hiểu khác biệt giữa tối ưu thuật toán và micro-optimizations (cache, inlining).

1. Đo và profiling

- Microbenchmark vs real workload: luôn test trên dữ liệu thực.
- Công cụ:
  - gprof: profiling cho chương trình được link với -pg (các hạn chế về inline).
  - perf (Linux): đo CPU cycles, cache-misses, branch-misses.
  - Valgrind Callgrind + kcachegrind: profile gọi hàm và số lần gọi.

2. Algorithmic optimization

- Đổi thuật toán thường đem lợi lớn nhất (O(n^2) -> O(n log n) ...).
- Trường hợp: dùng hash map thay vì tìm tuyến tính.

3. Micro-optimizations và cache-friendliness

- Data locality: tổ chức dữ liệu theo trình tự truy cập (AoS vs SoA).
- Prefetching, loop unrolling, inlining: cân nhắc trước khi áp dụng.
- Giữ critical loops đơn giản để compiler tối ưu.

4. Compiler optimizations and flags

- -O0, -O1, -O2, -O3, -Ofast, -march=native
- -flto (link-time optimization), -funroll-loops, -fprofile-guided
- Gợi ý: thử nhiều mức và profile để thấy tác động.

5. Benchmarking methodology

- Run multiple times, warm-up caches, pin CPU if cần.
- Use stable datasets and measure wall-clock and CPU time.

6. Ví dụ thực hành

- /code/chapter-17/sort_bench.c: so sánh quicksort vs mergesort vs std qsort với dữ liệu lớn và đo thời gian.
- /code/chapter-17/cache_demo.c: minh họa truy cập theo hàng vs cột trên matrix và đo cache effect.

Bài tập

1) Profile hàm tìm kiếm và thay bằng cấu trúc dữ liệu phù hợp; trình bày số liệu trước/sau.
2) Minh họa effect của memory layout bằng benchmark.

Ghi chú: kèm hướng dẫn chạy perf/gprof và cách đọc báo cáo.