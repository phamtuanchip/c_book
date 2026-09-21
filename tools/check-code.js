// Bien dich (va tuy chon chay) moi vi du trong code/ theo code/manifest.json.
//   CC="gcc" node tools/check-code.js                 # kiem tra bien dich
//   CC="zig cc -target x86_64-linux-musl" node tools/check-code.js   # bien dich cheo (khong chay)
// Bien moi truong: WARN=1 de coi canh bao la loi (-Werror).
'use strict';
const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const ROOT = path.join(__dirname, '..');
const CODE = path.join(ROOT, 'code');
const manifest = JSON.parse(fs.readFileSync(path.join(CODE, 'manifest.json'), 'utf8'));
const cc = (process.env.CC || 'gcc').split(/\s+/);
const flags = ['-std=c11', '-Wall', '-Wextra', '-O1', ...(process.env.WARN ? ['-Werror'] : [])];
const only = process.argv[2] ? String(parseInt(process.argv[2], 10)) : null;
const outDir = path.join(require('os').tmpdir(), 'c_book_check');
fs.mkdirSync(outDir, { recursive: true });

let fail = 0, ok = 0, warn = 0;
function run(dir, args, label) {
  const r = spawnSync(cc[0], [...cc.slice(1), ...flags, ...args], { cwd: dir, encoding: 'utf8' });
  const out = ((r.stdout || '') + (r.stderr || '')).trim();
  if (r.status !== 0) { fail++; console.log(`FAIL ${label}\n${out.split('\n').slice(0, 14).map((l) => '   ' + l).join('\n')}`); }
  else { ok++; if (out) { warn++; console.log(`WARN ${label}\n${out.split('\n').slice(0, 8).map((l) => '   ' + l).join('\n')}`); } }
}

for (const [ch, m] of Object.entries(manifest)) {
  if (only && !(ch === only || ch === 'solutions/' + String(only).padStart(2, '0'))) continue;
  const dir = ch.startsWith('solutions/') ? path.join(CODE, 'solutions', 'chapter-' + ch.split('/')[1]) : path.join(CODE, `chapter-${String(ch).padStart(2, '0')}`);
  const inc = (m.include || []).map((d) => '-I' + d);
  for (const p of m.programs) {
    const exe = path.join(outDir, `c${ch.replace(/[^a-zA-Z0-9]/g, "_")}_${p.out.replace(/\W/g, '_')}${process.platform === 'win32' ? '.exe' : ''}`);
    run(dir, [...inc, p.src, ...p.extra, '-o', exe, ...m.libs.filter(Boolean)], `ch${ch}/${p.src}`);
  }
  for (const c of m.compileOnly || []) run(dir, [...inc, '-c', c, '-o', path.join(outDir, 'x.o')], `ch${ch}/${c} (-c)`);
  if (m.project) {
    const exe = path.join(outDir, `c${ch.replace(/[^a-zA-Z0-9]/g, "_")}_app`);
    run(dir, ['-Iinclude', ...m.project.sources, '-o', exe, ...m.libs.filter(Boolean)], `ch${ch}/project`);
  }
}
console.log(`\n${ok} bien dich duoc (${warn} co canh bao), ${fail} loi.`);
process.exit(fail ? 1 : 0);
