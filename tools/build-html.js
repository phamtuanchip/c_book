// Build HTML tu manuscript/chapter-NN.md (+ solutions/) vao dist/ de publish GitHub Pages.
// Ban thao khong dung code fence (```), nen truoc khi render, script tu nhan dien cac khoi
// ma C (heuristic) va boc vao ```c. Cac khoi ``` co san duoc giu nguyen.
'use strict';

const fs = require('fs');
const path = require('path');
const MarkdownIt = require('markdown-it');
const hljs = require('highlight.js');

const ROOT = path.join(__dirname, '..');
const SRC = path.join(ROOT, 'manuscript');
const DIST = path.join(ROOT, 'dist');
const REPO = 'https://github.com/phamtuanchip/c_book';
const BOOK_TITLE = 'Lập trình C — Từ cơ bản đến nâng cao';
const BOOK_SUB = 'Bộ sách dạy lập trình C bằng tiếng Việt: từ chương trình đầu tiên đến con trỏ, bộ nhớ, đa luồng, socket và hai dự án nhỏ.';

const md = new MarkdownIt({
  html: false,
  linkify: true,
  highlight(str, lang) {
    const l = lang && hljs.getLanguage(lang) ? lang : null;
    const body = l ? hljs.highlight(str, { language: l }).value : md.utils.escapeHtml(str);
    return `<pre class="hljs"><code>${body}</code></pre>`;
  },
});

const isCodeLine = (l) => {
  const t = l.trim();
  if (!t) return false;
  if (/^[-*]\s|^\d+[.)]\s/.test(t)) return false; // muc danh sach luon la van xuoi
  if (/^#\s*(include|define|if|ifdef|ifndef|else|endif|pragma|undef)\b/.test(t)) return true;
  if (/^(\/\/|\/\*|\*\/|\*\s)/.test(t)) return true;
  if (/^[{}]/.test(t) || /[;{}]\s*$/.test(t)) return true;
  if (/^(else|do)\b/.test(t)) return true;
  if (/^\s{4,}\S/.test(l)) return true; // thut le >= 4 -> tiep tuc khoi ma
  return false;
};

function fenceCode(src) {
  const lines = src.split(/\r?\n/);
  const out = [];
  let inFence = false;
  for (let i = 0; i < lines.length; ) {
    const line = lines[i];
    if (/^```/.test(line)) { inFence = !inFence; out.push(line); i++; continue; }
    if (inFence) { out.push(line); i++; continue; }
    if (isCodeLine(line) && !/^\s{2,}[-*]\s/.test(line)) {
      // gom khoi: cac dong ma, cho phep dong trong neu dong ke tiep van la ma
      let j = i;
      let last = i;
      while (j < lines.length) {
        if (/^```/.test(lines[j])) break;
        if (isCodeLine(lines[j])) { last = j; j++; continue; }
        if (!lines[j].trim()) { j++; continue; }
        break;
      }
      out.push('', '```c', ...lines.slice(i, last + 1).map((l) => l.replace(/^\s{0,0}/, '')), '```', '');
      i = last + 1;
      continue;
    }
    // dong "N. Tieu de" dung rieng (co dong trong truoc va sau) -> tieu de muc
    const prevBlank = out.length === 0 || !out[out.length - 1].trim();
    const nextBlank = i + 1 >= lines.length || !lines[i + 1].trim();
    if (prevBlank && nextBlank && /^\d+\.\s+\S/.test(line)) out.push(`## ${line}`);
    else out.push(line);
    i++;
  }
  return out.join('\n');
}

function preprocess(src) {
  let s = fenceCode(src);
  // "/code/chapter-NN" -> link toi thu muc code tren GitHub
  s = s.replace(/(^|[^`\w])\/code\/(chapter-\d+)/g, (m, p, c) => `${p}[code/${c}](${REPO}/tree/main/code/${c})`);
  return s;
}

const files = fs.readdirSync(SRC).filter((f) => /^chapter-\d+\.md$/.test(f)).sort();
const chapters = files.map((f) => {
  const src = fs.readFileSync(path.join(SRC, f), 'utf8').replace(/^﻿/, '');
  const m = src.match(/^#\s+(.+)$/m);
  const num = parseInt(f.match(/\d+/)[0], 10);
  return { num, file: f, href: `chapter-${String(num).padStart(2, '0')}.html`, title: m ? m[1].replace(/^Chương\s+\d+\s*[—-]\s*/, '') : `Chương ${num}`, src };
});

const solDir = path.join(SRC, 'solutions');
const solutions = fs.existsSync(solDir)
  ? fs.readdirSync(solDir).filter((f) => /\.md$/.test(f)).sort().map((f) => {
      const src = fs.readFileSync(path.join(solDir, f), 'utf8').replace(/^﻿/, '');
      const n = parseInt(f.match(/\d+/)[0], 10);
      return { num: n, href: `solutions-${String(n).padStart(2, '0')}.html`, title: `Lời giải chương ${n}`, src };
    })
  : [];

const sidebar = (active) => {
  let h = `<nav class="sidebar"><div class="sidebar-title"><a href="index.html">${BOOK_TITLE}</a></div><ul class="part-list"><li class="part"><div class="part-title">Các chương</div><ul class="chapter-list">`;
  for (const c of chapters) h += `<li><a class="chapter-link${c.href === active ? ' active' : ''}" href="${c.href}">${c.num}. ${c.title}</a></li>`;
  h += '</ul></li>';
  if (solutions.length) {
    h += '<li class="part"><div class="part-title">Lời giải</div><ul class="chapter-list">';
    for (const s of solutions) h += `<li><a class="chapter-link${s.href === active ? ' active' : ''}" href="${s.href}">${s.title}</a></li>`;
    h += '</ul></li>';
  }
  h += `<li class="part"><div class="part-title">Mã nguồn</div><ul class="chapter-list"><li><a class="chapter-link" href="${REPO}/tree/main/code">Code mẫu trên GitHub</a></li></ul></li></ul></nav>`;
  return h;
};

const page = ({ title, body, href, prev, next }) => `<!doctype html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>${title} — ${BOOK_TITLE}</title>
<link rel="stylesheet" href="style.css">
</head>
<body>
<div class="layout">
${sidebar(href)}
<main class="content">
<article>
${body}
</article>
<div class="chapter-nav">${prev ? `<a class="nav-prev" href="${prev.href}">&larr; ${prev.title}</a>` : '<span></span>'}${next ? `<a class="nav-next" href="${next.href}">${next.title} &rarr;</a>` : '<span></span>'}</div>
</main>
</div>
</body>
</html>
`;

fs.rmSync(DIST, { recursive: true, force: true });
fs.mkdirSync(DIST, { recursive: true });
fs.copyFileSync(path.join(__dirname, 'style.css'), path.join(DIST, 'style.css'));

chapters.forEach((c, i) => {
  const html = page({ title: c.title, body: md.render(preprocess(c.src)), href: c.href, prev: chapters[i - 1], next: chapters[i + 1] });
  fs.writeFileSync(path.join(DIST, c.href), html, 'utf8');
});
solutions.forEach((s, i) => {
  const html = page({ title: s.title, body: md.render(preprocess(s.src)), href: s.href, prev: solutions[i - 1], next: solutions[i + 1] });
  fs.writeFileSync(path.join(DIST, s.href), html, 'utf8');
});

const indexBody = `<h1>${BOOK_TITLE}</h1><p class="subtitle">${BOOK_SUB}</p><p><a class="start-link" href="${chapters[0].href}">Bắt đầu đọc &rarr;</a></p><p>Mã nguồn và file EPUB/PDF: <a href="${REPO}">${REPO}</a></p>`;
fs.writeFileSync(path.join(DIST, 'index.html'), `<!doctype html>
<html lang="vi"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><title>${BOOK_TITLE}</title><link rel="stylesheet" href="style.css"></head>
<body><div class="layout">${sidebar('index.html')}<main class="content"><article>${indexBody}</article></main></div></body></html>
`, 'utf8');

console.log(`Da build ${chapters.length} chuong + ${solutions.length} trang loi giai vao dist/.`);
