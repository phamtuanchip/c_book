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
    if (prevBlank && nextBlank && line.length < 70 && !/[.:;,)]$/.test(line.trim()) && /^\d+\.\s+\S/.test(line)) out.push(`## ${line}`);
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

// ---- Bo cuc "quyen sach": cac phan, muc luc, dau chuong, dieu huong ----
const PARTS = [
  { title: 'Phần I — Nhập môn', from: 1, to: 4 },
  { title: 'Phần II — Nền tảng ngôn ngữ', from: 5, to: 8 },
  { title: 'Phần III — Bộ nhớ, dữ liệu, xây dựng', from: 9, to: 13 },
  { title: 'Phần IV — Kỹ thuật nâng cao', from: 14, to: 18 },
  { title: 'Phần V — Dự án và phát hành', from: 19, to: 22 },
];
const partOf = (n) => PARTS.find((p) => n >= p.from && n <= p.to);
const esc = (s) => s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');

const sidebar = (active) => {
  let h = `<nav class="sidebar" id="sidebar"><div class="sidebar-title"><a href="index.html">${BOOK_TITLE}</a></div><ul class="part-list">`;
  for (const p of PARTS) {
    h += `<li class="part"><div class="part-title">${p.title}</div><ul class="chapter-list">`;
    for (const c of chapters.filter((x) => x.num >= p.from && x.num <= p.to))
      h += `<li><a class="chapter-link${c.href === active ? ' active' : ''}" href="${c.href}">${c.num}. ${esc(c.title)}</a></li>`;
    h += '</ul></li>';
  }
  if (solutions.length) {
    h += '<li class="part"><div class="part-title">Phụ lục — Lời giải</div><ul class="chapter-list">';
    for (const s of solutions) h += `<li><a class="chapter-link${s.href === active ? ' active' : ''}" href="${s.href}">${esc(s.title)}</a></li>`;
    h += '</ul></li>';
  }
  h += `<li class="part"><div class="part-title">Mã nguồn</div><ul class="chapter-list"><li><a class="chapter-link" href="${REPO}/tree/main/code">Code mẫu trên GitHub</a></li></ul></li></ul></nav>`;
  return h;
};

// Them id cho h2/h3 ("5.2. Ten" -> s5-2) va tra ve danh sach muc de dung muc luc chuong
function addHeadingIds(html) {
  const toc = [];
  let counter = 0;
  const out = html.replace(/<h([23])>([\s\S]*?)<\/h\1>/g, (m, lvl, inner) => {
    const text = inner.replace(/<[^>]+>/g, '');
    const num = text.match(/^(\d+)\.(\d+)/);
    const id = num ? `s${num[1]}-${num[2]}` : `h-${++counter}`;
    if (lvl === '2') toc.push({ id, text });
    return `<h${lvl} id="${id}">${inner}</h${lvl}>`;
  });
  return { html: out, toc };
}

function renderArticle(src) {
  let html = md.render(preprocess(src));
  const { html: withIds, toc } = addHeadingIds(html);
  html = withIds;
  // Tieu de h1 "Chuong N — Ten" -> dau chuong
  html = html.replace(/<h1>Chương\s+(\d+)\s*[—-]\s*([\s\S]*?)<\/h1>/, (m, n, t) =>
    `<header class="chapter-head"><div class="chapter-num">Chương ${n}</div><h1>${t}</h1></header>`);
  // Muc luc trong chuong (chi khi co tu 4 muc tro len)
  if (toc.length >= 4) {
    const items = toc.filter((t) => !/^Mục tiêu/.test(t.text)).map((t) => `<li><a href="#${t.id}">${t.text}</a></li>`).join('');
    const box = `<nav class="chapter-toc" aria-label="Trong chương này"><div class="toc-title">Trong chương này</div><ol>${items}</ol></nav>`;
    html = html.replace('</header>', `</header>${box}`);
  }
  // Bao bang de cuon ngang tren man hinh nho
  html = html.replace(/<table>/g, '<div class="table-wrap"><table>').replace(/<\/table>/g, '</table></div>');
  return html;
}

const THEME_INIT = `<script>try{var t=localStorage.getItem('theme');if(!t&&window.matchMedia&&matchMedia('(prefers-color-scheme: dark)').matches)t='dark';if(t)document.documentElement.setAttribute('data-theme',t);var f=localStorage.getItem('fs');if(f)document.documentElement.style.setProperty('--fs',f+'px')}catch(e){}</script>`;

const PAGE_JS = `<script>
(function(){
  var root=document.documentElement,store=function(k,v){try{localStorage.setItem(k,v)}catch(e){}};
  function setTheme(t){if(t==='light')root.removeAttribute('data-theme');else root.setAttribute('data-theme',t);store('theme',t);
    document.querySelectorAll('[data-theme-btn]').forEach(function(b){b.setAttribute('aria-pressed',String((b.dataset.themeBtn===t)||(t==='light'&&b.dataset.themeBtn==='light')))})}
  var cur=root.getAttribute('data-theme')||'light';setTheme(cur);
  document.querySelectorAll('[data-theme-btn]').forEach(function(b){b.addEventListener('click',function(){setTheme(b.dataset.themeBtn)})});
  var fs=parseInt(getComputedStyle(root).getPropertyValue('--fs'))||19;
  document.querySelectorAll('[data-fs]').forEach(function(b){b.addEventListener('click',function(){fs=Math.min(26,Math.max(14,fs+parseInt(b.dataset.fs)));root.style.setProperty('--fs',fs+'px');store('fs',fs)})});
  var mb=document.getElementById('menu-btn');if(mb)mb.addEventListener('click',function(){document.body.classList.toggle('nav-open')});
  document.addEventListener('click',function(e){if(document.body.classList.contains('nav-open')&&!e.target.closest('#sidebar')&&e.target.id!=='menu-btn')document.body.classList.remove('nav-open')});
  document.querySelectorAll('pre').forEach(function(pre){var b=document.createElement('button');b.className='copy-btn';b.type='button';b.textContent='Sao chép';
    b.addEventListener('click',function(){var t=pre.querySelector('code')?pre.querySelector('code').innerText:pre.innerText;
      (navigator.clipboard?navigator.clipboard.writeText(t):Promise.reject()).then(function(){b.textContent='Đã chép'},function(){b.textContent='Lỗi'});setTimeout(function(){b.textContent='Sao chép'},1500)});pre.appendChild(b)});
  var a=document.querySelector('.chapter-link.active');if(a&&a.scrollIntoView)a.scrollIntoView({block:'center'});
})();
</script>`;

const topbar = (crumb) => `<header class="topbar"><button class="menu-btn" id="menu-btn" type="button" aria-label="Mục lục">☰</button><span class="crumb">${crumb}</span><span class="tools"><button data-fs="-1" type="button" title="Chữ nhỏ hơn">A−</button><button data-fs="1" type="button" title="Chữ lớn hơn">A+</button><button data-theme-btn="light" type="button" title="Nền sáng">Sáng</button><button data-theme-btn="sepia" type="button" title="Nền sepia">Sepia</button><button data-theme-btn="dark" type="button" title="Nền tối">Tối</button></span></header>`;

const shell = ({ title, crumb, body, active, footer = '' }) => `<!doctype html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>${title}</title>
${THEME_INIT}
<link rel="stylesheet" href="style.css">
</head>
<body>
<div class="layout">
${sidebar(active)}
<div class="main-col">
${topbar(crumb)}
<main class="content">
<article>
${body}
</article>
${footer}
</main>
</div>
</div>
${PAGE_JS}
</body>
</html>
`;

const navFooter = (prev, next) => `<nav class="chapter-nav" aria-label="Điều hướng chương">${prev ? `<a class="nav-prev" href="${prev.href}"><small>← Trước</small>${esc(prev.title)}</a>` : '<span></span>'}${next ? `<a class="nav-next" href="${next.href}"><small>Tiếp →</small>${esc(next.title)}</a>` : '<span></span>'}</nav>`;

fs.rmSync(DIST, { recursive: true, force: true });
fs.mkdirSync(DIST, { recursive: true });
fs.copyFileSync(path.join(__dirname, 'style.css'), path.join(DIST, 'style.css'));
fs.writeFileSync(path.join(DIST, '.nojekyll'), '');

chapters.forEach((c, i) => {
  const p = partOf(c.num);
  fs.writeFileSync(path.join(DIST, c.href), shell({
    title: `${c.title} — ${BOOK_TITLE}`,
    crumb: `${p ? p.title.split(' — ')[0] + ' · ' : ''}Chương ${c.num}`,
    body: renderArticle(c.src), active: c.href, footer: navFooter(chapters[i - 1], chapters[i + 1]),
  }), 'utf8');
});
solutions.forEach((s, i) => {
  fs.writeFileSync(path.join(DIST, s.href), shell({
    title: `${s.title} — ${BOOK_TITLE}`, crumb: s.title,
    body: renderArticle(s.src), active: s.href, footer: navFooter(solutions[i - 1], solutions[i + 1]),
  }), 'utf8');
});

let tocHtml = '<section class="toc"><h2>Mục lục</h2>';
for (const p of PARTS) {
  tocHtml += `<div class="toc-part">${p.title}</div><ol>`;
  for (const c of chapters.filter((x) => x.num >= p.from && x.num <= p.to))
    tocHtml += `<li><a href="${c.href}"><span class="n">${c.num}</span><span>${esc(c.title)}</span></a></li>`;
  tocHtml += '</ol>';
}
if (solutions.length) {
  tocHtml += '<div class="toc-part">Phụ lục — Lời giải bài tập</div><ol>';
  for (const s of solutions) tocHtml += `<li><a href="${s.href}"><span class="n">↳</span><span>${esc(s.title)}</span></a></li>`;
  tocHtml += '</ol>';
}
tocHtml += '</section>';

const cover = `<div class="cover"><div class="kicker">Giáo trình</div><h1>${BOOK_TITLE}</h1><p class="subtitle">${BOOK_SUB}</p><div class="ornament">◆ ◆ ◆</div><a class="start-link" href="${chapters[0].href}">Bắt đầu đọc →</a><p style="margin-top:1.4rem;font-size:.8rem;font-family:var(--font-ui);color:var(--muted)">Mã nguồn, EPUB và PDF: <a href="${REPO}">${REPO.replace('https://', '')}</a></p></div>`;
fs.writeFileSync(path.join(DIST, 'index.html'), shell({
  title: BOOK_TITLE, crumb: 'Mục lục', body: cover + tocHtml, active: 'index.html',
}), 'utf8');

console.log(`Da build ${chapters.length} chuong + ${solutions.length} trang loi giai vao dist/.`);
