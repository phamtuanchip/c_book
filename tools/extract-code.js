// Trich cac vi du hoan chinh tu manuscript/chapter-NN.md ra code/chapter-NN/ de sach va ma nguon luon khop nhau.
//
// Quy uoc trong ban thao: khoi ```c co dong dau la "// ten_file.c" (hoac .h, hoac "// thu_muc/ten.c")
// duoc xem la mot file hoan chinh. Khoi co ghi chu "(phan chinh", "(y tuong", "(rut gon" bi bo qua.
// Mot so chuong duoc ghep tu nhieu khoi (calc.c o chuong 19, server.c o chuong 20).
//
// Dung: node tools/extract-code.js        (ghi vao code/)
'use strict';

const fs = require('fs');
const path = require('path');

const ROOT = path.join(__dirname, '..');
const SRC = path.join(ROOT, 'manuscript');
const OUT = path.join(ROOT, 'code');
const REPO = 'https://github.com/phamtuanchip/c_book';
const TITLES = {};

// ---- Doc cac khoi ma ----
function parseBlocks(src) {
  const lines = src.split(/\r?\n/);
  const blocks = [];
  let section = '';
  for (let i = 0; i < lines.length; i++) {
    const h = lines[i].match(/^##\s+(\d+\.\d+)\./);
    if (h) section = h[1];
    const f = lines[i].match(/^```(\w*)\s*$/);
    if (f && f[1] !== undefined && !(blocks.open)) {
      const lang = f[1];
      const body = [];
      i++;
      while (i < lines.length && !/^```\s*$/.test(lines[i])) body.push(lines[i++]);
      blocks.push({ lang, section, text: body.join('\n') });
    }
  }
  return blocks;
}

// ---- Mo ta ngan cho README ----
const DESC = {
  'hello.c': 'Chương trình đầu tiên: in "Hello, World!"',
  'hello_name.c': 'Đọc tên bằng fgets và chào',
  'arithmetic.c': 'Bốn phép toán với hai số nguyên, xử lý chia cho 0',
  'bases.c': 'In một số ở hệ thập phân, hex, bát phân',
  'overflow.c': 'Tràn số có dấu / không dấu',
  'float_trap.c': 'Bẫy so sánh số thực bằng ==',
  'char_is_number.c': 'Ký tự là số nguyên nhỏ; đổi chữ hoa/thường, ký tự số',
  'sizes.c': 'In sizeof các kiểu cơ bản',
  'endian.c': 'Xác định endianness bằng cách in từng byte',
  'memory_regions.c': 'Địa chỉ của biến ở các vùng data/bss/heap/stack',
  'add.c': 'Hàm cộng đơn giản để xem assembly (`gcc -S`)',
  'bin_convert.c': 'In số nguyên dưới dạng nhị phân 32 bit',
  'ascii_table.c': 'In bảng ASCII in được',
  'demo_malloc.c': 'malloc, realloc, free đúng cách',
  'compat_test.c': 'Thử biên dịch với các chuẩn C khác nhau',
  'ub_demo.c': 'Undefined behavior thay đổi theo mức tối ưu',
  'read_int.c': 'Đọc số nguyên an toàn bằng fgets + strtol',
  'classify.c': 'Phân loại số âm/dương/0, chẵn/lẻ',
  'sum_input.c': 'Cộng các số nhập đến hết đầu vào',
  'prime_check.c': 'Kiểm tra số nguyên tố',
  'menu.c': 'Menu tương tác dùng switch và do-while',
  'counter.c': 'Biến static ở phạm vi file (module)',
  'text_stats.c': 'Đếm ký tự, từ, tần suất chữ cái',
  'ptr_practice.c': 'Đảo mảng và tìm chuỗi con bằng con trỏ',
  'leak_demo.c': 'Rò rỉ bộ nhớ có chủ đích (dùng Valgrind/ASan để bắt)',
  'use_after_free.c': 'Use-after-free có chủ đích (ASan)',
  'vec.c': 'Mảng động (Vec) với init/push/destroy',
  'tree.c': 'Cây nhị phân tìm kiếm, giải phóng đúng thứ tự',
  'layout.c': 'sizeof/offsetof và padding của struct',
  'linked_list.c': 'Danh sách liên kết đơn',
  'stack.c': 'Ngăn xếp bằng mảng động',
  'queue.c': 'Hàng đợi vòng (ring buffer)',
  'stack_adt.h': 'Giao diện ADT stack (opaque pointer)',
  'stack_adt.c': 'Cài đặt ADT stack',
  'write_text.c': 'Ghi file văn bản',
  'print_lines.c': 'Đọc file theo dòng, đánh số dòng',
  'copy_file.c': 'Sao chép file nhị phân bằng fread/fwrite',
  'csv_parser.c': 'Tách một dòng CSV có trường trong dấu nháy kép',
  'wc_lite.c': 'Đếm dòng/từ/byte như wc',
  'mini_grep.c': 'Tìm chuỗi trong file như grep',
  'merge_files.c': 'Nối nhiều file vào một file',
  'log.h': 'Logging: macro LOG_* và log_write',
  'log.c': 'Cài đặt log_write',
  'fault.h': 'Chèn lỗi cấp phát (fault injection)',
  'fault.c': 'Cài đặt test_malloc',
  'sum_file.c': 'Tính tổng số trong file, xử lý lỗi đầy đủ',
  'tiny_test.h': 'Khung kiểm thử nhỏ dựa trên macro',
  'test_math.c': 'Ví dụ dùng tiny_test.h',
  'hello_threads.c': 'Tạo và chờ nhiều luồng',
  'parallel_sum.c': 'Cộng mảng lớn bằng nhiều luồng',
  'race.c': 'Race condition và cách sửa bằng mutex',
  'producer_consumer.c': 'Producer–consumer với hàng đợi giới hạn',
  'echo_server.c': 'TCP echo server (tuần tự)',
  'echo_client.c': 'TCP client đọc từ bàn phím',
  'http_get.c': 'Gửi HTTP GET thô bằng getaddrinfo',
  'timer.h': 'Đồng hồ đo thời gian (clock_gettime)',
  'profile_demo.c': 'Chương trình có điểm nghẽn O(n²) để profile',
  'cache_demo.c': 'Duyệt ma trận theo hàng và theo cột',
  'sort_bench.c': 'So sánh qsort, quicksort, mergesort',
  'vuln.c': 'Tràn bộ đệm có chủ đích (chỉ để học, dùng với ASan)',
  'safe_parse.c': 'Phân tích thông điệp có tiền tố độ dài an toàn',
  'calc.c': 'Trình thông dịch mini: lexer → parser → AST → evaluator/VM (REPL)',
  'server.c': 'Web server: thread pool, file tĩnh, JSON API, tắt êm',
  'stats.h': 'Lõi thống kê thuần (dễ kiểm thử)',
  'stats.c': 'Cài đặt stats_compute',
  'mini_test.h': 'Khung kiểm thử: ASSERT_*, RUN_TEST',
  'test_stats.c': 'Bộ kiểm thử cho stats',
  'test_stats_unity.c': 'Bộ kiểm thử dùng Unity (cần tải Unity, xem README)',
  'geometry.h': 'Header giao diện module hình học',
  'geometry.c': 'Cài đặt module hình học',
  'main.c': 'Chương trình chính',
  'math_utils.h': 'Header của module toán',
  'math_utils.c': 'Cài đặt module toán',
};

// Chuong/chuong trinh can them nguon hoac co co bien dich rieng
const EXTRA_SOURCES = { 'test_stats.c': ['stats.c'] };
const PTHREAD = new Set([15, 16, 20]);
const NEEDS_UNITY = new Set(['test_stats_unity.c']);
const NOT_BUILT = new Set(['vuln.c']); // co tinh co loi; chi build qua target rieng

function firstLineName(text) {
  const first = text.split('\n')[0];
  const m = first.match(/^\/\/\s*([\w./-]+\.[ch])\b(.*)$/);
  if (!m) return null;
  if (/\((phần chính|ý tưởng|rút gọn|một phần)/.test(m[2]) || /dùng lại/.test(m[2])) return null;
  return m[1];
}

function normalizeFirstLine(text, name) {
  const lines = text.split('\n');
  lines[0] = `// ${name}`;
  return lines.join('\n').replace(/\s+$/, '') + '\n';
}

const written = {}; // chapter -> [{name, isProgram}]
function put(ch, rel, content) {
  const p = path.join(OUT, `chapter-${String(ch).padStart(2, '0')}`, rel);
  fs.mkdirSync(path.dirname(p), { recursive: true });
  fs.writeFileSync(p, content, 'utf8');
  written[ch] = written[ch] || []; if (!written[ch].includes(rel)) written[ch].push(rel);
}

// ---- Don sach thu muc cu (giu file khong phai ma nguon nhu release_script.ps1) ----
function cleanChapterDir(ch) {
  const d = path.join(OUT, `chapter-${String(ch).padStart(2, '0')}`);
  if (!fs.existsSync(d)) return;
  for (const e of fs.readdirSync(d, { withFileTypes: true })) {
    const full = path.join(d, e.name);
    if (e.isDirectory()) fs.rmSync(full, { recursive: true, force: true });
    else if (/\.(c|h|txt|md)$|^Makefile$|^CMakeLists\.txt$/.test(e.name)) fs.rmSync(full);
  }
}

const files = fs.readdirSync(SRC).filter((f) => /^chapter-\d+\.md$/.test(f)).sort();
const manifest = {};

for (const f of files) {
  const ch = parseInt(f.match(/\d+/)[0], 10);
  const src = fs.readFileSync(path.join(SRC, f), 'utf8');
  TITLES[ch] = (src.match(/^#\s+(.+)$/m) || [, `Chương ${ch}`])[1];
  cleanChapterDir(ch);
  const blocks = parseBlocks(src);
  const seen = new Set();

  for (const b of blocks) {
    if (b.lang === 'c') {
      const name = firstLineName(b.text);
      if (name && !seen.has(name)) {
        seen.add(name);
        put(ch, name, normalizeFirstLine(b.text, name));
      }
    }
  }

  // ---- Ghep nhieu khoi thanh mot chuong trinh ----
  if (ch === 19) {
    const parts = blocks.filter((b) => b.lang === 'c' && parseFloat(b.section.replace('.', '.')) >= 19.3 &&
      !/static const char \*tok_name|static void ast_print|^\/\/ test_calc/m.test(b.text) &&
      !/^\/\/ (test_calc)/.test(b.text) && b.section >= '19.3');
    const inRange = parts.filter((b) => ['19.3', '19.4', '19.5', '19.6', '19.7'].includes(b.section));
    const code = ['// calc.c — trình thông dịch mini (ghép từ các mục 19.3–19.7)', '#include <stdio.h>', '#include <stdlib.h>', '#include <string.h>', '#include <ctype.h>', '']
      .concat(inRange.map((b) => b.text.replace(/^\/\/ [\w.]+\.c[^\n]*\n/, '')))
      .join('\n\n') + '\n';
    put(ch, 'calc.c', code);
  }
  if (ch === 20) {
    const inRange = blocks.filter((b) => b.lang === 'c' && /^20\.(3|4|5|6|7|8|9|10)$/.test(b.section));
    const code = inRange.map((b, i) => (i === 0 ? b.text : b.text)).join('\n\n') + '\n';
    put(ch, 'server.c', code.replace(/^\/\/ server\.c\n/, '// server.c — web server học tập (ghép từ các mục 20.3–20.10)\n'));
    const html = blocks.find((b) => b.lang === 'html');
    if (html) put(ch, 'www/index.html', html.text + '\n');
    put(ch, 'www/style.css', 'body { font-family: system-ui, sans-serif; max-width: 40rem; margin: 3rem auto; padding: 0 1rem; line-height: 1.6; }\nh1 { color: #8b2e2e; }\n');
    const mk = blocks.find((b) => b.lang === 'make' && /server: server\.c/.test(b.text));
    if (mk) put(ch, 'Makefile', mk.text + '\n');
  }
  if (ch === 11) {
    const mk = blocks.find((b) => b.lang === 'make' && /Cấu hình/.test(b.text));
    if (mk) put(ch, 'Makefile', mk.text + '\n');
    const cm = blocks.find((b) => b.lang === 'cmake' && /geometry_demo/.test(b.text));
    if (cm) put(ch, 'CMakeLists.txt', cm.text + '\n');
  }
  if (ch === 5) { /* khong co ghep */ }
}

// ---- Makefile + README + manifest cho tung chuong ----
for (const f of files) {
  const ch = parseInt(f.match(/\d+/)[0], 10);
  const dir = path.join(OUT, `chapter-${String(ch).padStart(2, '0')}`);
  if (!fs.existsSync(dir)) continue;
  const rels = (written[ch] || []).filter((r) => /\.[ch]$/.test(r));
  const hasSrcDir = rels.some((r) => r.startsWith('src/'));
  const list = [];
  const progs = [];
  for (const r of rels) {
    if (r.startsWith('www/')) continue;
    const txt = fs.readFileSync(path.join(dir, r), 'utf8');
    const isMain = /\bint\s+main\s*\(/.test(txt);
    const base = path.basename(r);
    list.push({ rel: r, isMain, desc: DESC[base] || '' });
    if (isMain && !hasSrcDir && !NEEDS_UNITY.has(base) && !NOT_BUILT.has(base)) progs.push(r);
  }

  const pthread = PTHREAD.has(ch) ? ' -pthread' : '';
  const ldlibs = `-lm${pthread}`;
  const extras = {};
  for (const p of progs) if (EXTRA_SOURCES[p]) extras[p] = EXTRA_SOURCES[p];

  manifest[ch] = {
    programs: progs.map((p) => ({ src: p, extra: extras[p] || [], out: p.replace(/\.c$/, '') })),
    libs: ldlibs.split(' '),
    project: hasSrcDir ? { sources: rels.filter((r) => r.startsWith('src/')), include: 'include' } : null,
    compileOnly: list.filter((x) => !x.isMain && x.rel.endsWith('.c') && !x.rel.startsWith('src/')).map((x) => x.rel),
  };

  if (!fs.existsSync(path.join(dir, 'Makefile'))) {
    let mk = `# Build các ví dụ của chương ${ch}. Dùng: make | make asan | make clean\n`;
    mk += 'CC      ?= gcc\nCFLAGS  ?= -std=c11 -Wall -Wextra -g -O1\n';
    if (hasSrcDir) {
      mk += `LDLIBS  := ${ldlibs}\nSRCS := $(wildcard src/*.c)\n\n.PHONY: all asan clean run\nall: build/app\n\nbuild/app: $(SRCS) | build\n\t$(CC) $(CFLAGS) -Iinclude $(SRCS) -o $@ $(LDLIBS)\n\nbuild:\n\tmkdir -p build\n\nrun: build/app\n\t./build/app\n\nasan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer\nasan: clean all\n\nclean:\n\trm -rf build\n`;
    } else {
      const names = progs.map((p) => p.replace(/\.c$/, ''));
      mk += `LDLIBS  := ${ldlibs}\nPROGS   := ${names.join(' ')}\n\n.PHONY: all asan clean\nall: $(PROGS)\n\n%: %.c\n\t$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)\n\n`;
      for (const p of progs) if (extras[p]) mk += `${p.replace(/\.c$/, '')}: ${p} ${extras[p].join(' ')}\n\t$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)\n\n`;
      mk += 'asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer\nasan: clean all\n\nclean:\n\trm -f $(PROGS) *.exe\n';
      if (!progs.length) mk = `# Chương ${ch} chỉ có các file ví dụ/header; không có chương trình để build riêng.\nall:\n\t@echo "Không có chương trình độc lập trong chương này."\n`;
    }
    fs.writeFileSync(path.join(dir, 'Makefile'), mk, 'utf8');
  }

  const chapNum = String(ch).padStart(2, '0');
  let rd = `# Mã nguồn chương ${ch}\n\n${TITLES[ch] || ''}\n\n`;
  rd += `Các file dưới đây được **trích tự động từ bản thảo** (\`manuscript/chapter-${chapNum}.md\`) bằng \`node tools/extract-code.js\`, nên luôn khớp với nội dung sách. Đừng sửa trực tiếp ở đây — hãy sửa trong bản thảo rồi chạy lại script.\n\n`;
  if (list.length) {
    rd += '## Các file\n\n| File | Mô tả |\n|---|---|\n';
    for (const x of list) rd += `| \`${x.rel}\` | ${x.desc || (x.isMain ? 'Chương trình ví dụ' : 'Module / header dùng chung')} |\n`;
    rd += '\n';
  } else {
    rd += 'Chương này chủ yếu là lý thuyết và các đoạn mã ngắn trong sách nên không có chương trình độc lập; hãy gõ lại các đoạn mã trong sách và thử nghiệm.\n\n';
  }
  rd += '## Biên dịch và chạy\n\n```bash\nmake            # build tất cả\nmake asan       # build với AddressSanitizer + UBSan (Linux/macOS/WSL)\nmake clean\n```\n\n';
  if (progs.length && !hasSrcDir) rd += `Hoặc thủ công, ví dụ:\n\n\`\`\`bash\ngcc -std=c11 -Wall -Wextra -g -o ${progs[0].replace(/\.c$/, '')} ${progs[0]} ${ldlibs}\n\`\`\`\n\n`;
  const posix = [15, 16, 20].includes(ch);
  if (posix) rd += '> **Lưu ý:** mã dùng POSIX (pthread, socket). Chạy trên Linux, macOS hoặc **WSL**; không biên dịch trực tiếp bằng MinGW/MSVC nếu chưa chuyển sang Winsock (xem chương 16).\n\n';
  if (ch === 19) rd += '## Chạy thử\n\n```bash\n./calc            # REPL dùng evaluator duyệt cây\n./calc --vm       # REPL dùng bytecode VM\n> x = 1 + 2 * (3 - 4)\n= -1\n```\n\n';
  if (ch === 20) rd += '## Chạy thử\n\n```bash\nmake && ./server 8080 4 ./www\ncurl -i http://127.0.0.1:8080/\ncurl -i http://127.0.0.1:8080/api/time\n```\n\n';
  if (files.length && list.some((x) => NEEDS_UNITY.has(path.basename(x.rel)))) rd += '`test_stats_unity.c` cần thư viện [Unity](https://github.com/ThrowTheSwitch/Unity): chép `unity.c`, `unity.h`, `unity_internals.h` vào thư mục này rồi `gcc test_stats_unity.c stats.c unity.c -o test_unity`.\n\n';
  if (ch === 18) rd += '`vuln.c` **có lỗi cố ý**. Chỉ biên dịch bằng `make vuln` (không nằm trong `make all`) và chạy với `-fsanitize=address` trên máy của bạn.\n\n';
  rd += `Đọc lại chương: ${REPO}/blob/main/manuscript/chapter-${chapNum}.md\n`;
  fs.writeFileSync(path.join(dir, 'README.md'), rd, 'utf8');
}

// ---- Ma trong loi giai: manuscript/solutions/chapter-NN-solutions.md -> code/solutions/chapter-NN/ ----
const SOLDIR = path.join(SRC, 'solutions');
if (fs.existsSync(SOLDIR)) {
  fs.rmSync(path.join(OUT, 'solutions'), { recursive: true, force: true });
  const NL = '\n';
  let readme = '# Mã nguồn lời giải' + NL + NL +
    'Các file dưới đây được **trích tự động** từ `manuscript/solutions/chapter-NN-solutions.md` (`node tools/extract-code.js`) ' +
    'và được kiểm tra biên dịch bằng `node tools/check-code.js`.' + NL + NL;
  for (const f of fs.readdirSync(SOLDIR).filter((x) => /^chapter-\d+-solutions\.md$/.test(x)).sort()) {
    const ch = parseInt(f.match(/\d+/)[0], 10);
    const cd = String(ch).padStart(2, '0');
    const blocks = parseBlocks(fs.readFileSync(path.join(SOLDIR, f), 'utf8'));
    const seen = new Set();
    const rels = [];
    for (const b of blocks) {
      if (b.lang !== 'c') continue;
      const name = firstLineName(b.text);
      if (!name || seen.has(name)) continue;
      seen.add(name);
      const p = path.join(OUT, 'solutions', 'chapter-' + cd, name);
      fs.mkdirSync(path.dirname(p), { recursive: true });
      fs.writeFileSync(p, normalizeFirstLine(b.text, name), 'utf8');
      rels.push({ name, isMain: /\bint\s+main\s*\(/.test(b.text) });
    }
    if (!rels.length) continue;
    manifest['solutions/' + cd] = {
      programs: rels.filter((r) => r.isMain && r.name.endsWith('.c')).map((r) => ({ src: r.name, extra: r.name === 'calc_main.c' ? ['calc.c'] : [], out: r.name.replace(/\.c$/, '') })),
      libs: ['-lm'].concat(PTHREAD.has(ch) ? ['-pthread'] : []),
      compileOnly: rels.filter((r) => !r.isMain && r.name.endsWith('.c')).map((r) => r.name),
      include: ['../../chapter-' + cd],
    };
    readme += '## Chương ' + ch + NL + NL + rels.map((r) => '- `chapter-' + cd + '/' + r.name + '`').join(NL) + NL + NL;
  }
  fs.writeFileSync(path.join(OUT, 'solutions', 'README.md'), readme, 'utf8');
}

fs.writeFileSync(path.join(OUT, 'manifest.json'), JSON.stringify(manifest, null, 2) + '\n');
let total = 0;
for (const ch of Object.keys(written)) total += written[ch].length;
console.log(`Da trich ${total} file vao code/ (${Object.keys(written).length} chuong).`);
