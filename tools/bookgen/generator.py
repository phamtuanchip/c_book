"""Cross-platform book generator utilities.
- Collect manuscript files
- Run pandoc to build EPUB/PDF
- Generate simple cover (Pillow optional)
- Package code samples
"""
from pathlib import Path
import subprocess
import shutil
import zipfile
import unicodedata
import re

class BookGenerator:
    def __init__(self, repo_root: str | Path):
        self.root = Path(repo_root).resolve()
        self.manuscript = self.root / 'manuscript'
        self.formats = self.root / 'formats'
        self.fonts = self.formats / 'fonts'
        self.kdp = self.root / 'kdp'

    def collect_markdown(self):
        md = sorted(self.manuscript.glob('*.md'))
        if not md:
            raise FileNotFoundError('No markdown files found in manuscript/')
        return [str(p) for p in md]

    def _find_pandoc(self, hint_paths=None):
        # Try shutil.which first, then common Windows user install locations
        p = shutil.which('pandoc')
        if p:
            return p
        hint_paths = hint_paths or [
            str(Path.home() / 'AppData' / 'Local' / 'Pandoc' / 'pandoc.exe'),
            str(Path('C:/Program Files/Pandoc/pandoc.exe'))
        ]
        for hp in hint_paths:
            if Path(hp).exists():
                return hp
        raise FileNotFoundError('pandoc executable not found on PATH or common locations')

    def _find_cover(self):
        for name in ('cover.jpg', 'cover.png'):
            p = self.formats / name
            if p.exists():
                return p
        cover_kdp = self.kdp / 'cover.jpg'
        if cover_kdp.exists():
            return cover_kdp
        return None

    def build_epub(self, out: str | Path = None, pandoc_cmd: str | None = None, css: str | None = None, metadata: str | None = None, embed_fonts: bool = True, cover: str | None = None):
        if pandoc_cmd is None:
            pandoc_cmd = self._find_pandoc()
        out = Path(out or self.formats / 'book.epub')
        # auto-detect cover if not specified
        if cover is None:
            cover = self._find_cover()
        args = [pandoc_cmd]
        args += self.collect_markdown()
        args += ['--from', 'markdown+fenced_code_attributes', '--toc', '--to', 'epub3']
        if metadata:
            args += ['--metadata-file', str(metadata)]
        if css:
            args += ['--css', str(css)]
        if cover:
            args += ['--epub-cover-image', str(cover)]
        if embed_fonts and self.fonts.exists():
            for f in sorted(self.fonts.glob('*.ttf')):
                args += ['--epub-embed-font', str(f)]
        args += ['-o', str(out)]
        print('Running:', ' '.join(args))
        subprocess.check_call(args)
        self._fix_epub_cover(out)
        return out

    def _fix_epub_cover(self, epub_path: Path):
        """Replace pandoc's SVG cover wrapper with a plain <img> tag.

        Pandoc wraps --epub-cover-image in an SVG with xlink:href (deprecated),
        which causes Kindle's internal converter to fail. This post-processes
        the EPUB in-place to use a simple <img> element instead.
        """
        import tempfile, os
        tmp = epub_path.with_suffix('.tmp.epub')
        with zipfile.ZipFile(epub_path, 'r') as zin, \
             zipfile.ZipFile(tmp, 'w', compression=zipfile.ZIP_DEFLATED) as zout:
            for item in zin.infolist():
                data = zin.read(item.filename)
                if item.filename == 'EPUB/text/cover.xhtml':
                    data = self._rewrite_cover_xhtml(data.decode('utf-8')).encode('utf-8')
                elif item.filename == 'EPUB/content.opf':
                    data = data.decode('utf-8').replace(
                        'properties="svg" ', ''
                    ).replace(
                        ' properties="svg"', ''
                    ).encode('utf-8')
                zout.writestr(item, data)
        os.replace(tmp, epub_path)

    def _rewrite_cover_xhtml(self, xhtml: str) -> str:
        """Swap the SVG block for a plain <img> so Kindle KFX can convert it."""
        import re
        # extract image href from the SVG <image> element
        m = re.search(r'<image[^>]+href=["\']([^"\']+)["\']', xhtml)
        if not m:
            return xhtml
        img_src = m.group(1)
        # build a minimal cover page with plain <img>
        return (
            '<?xml version="1.0" encoding="UTF-8"?>\n'
            '<!DOCTYPE html>\n'
            '<html xmlns="http://www.w3.org/1999/xhtml" '
            'xmlns:epub="http://www.idpf.org/2007/ops">\n'
            '<head><meta charset="utf-8"/><title>Cover</title>'
            '<style>body{margin:0;padding:0;text-align:center;}'
            'img{max-width:100%;height:auto;}</style></head>\n'
            '<body epub:type="cover">'
            f'<img src="{img_src}" alt="cover"/>'
            '</body>\n</html>\n'
        )

    def _find_tex_engine(self, engine_name: str):
        # Try to locate a TeX engine executable (xelatex, lualatex) using PATH or common MiKTeX location
        p = shutil.which(engine_name)
        if p:
            return p
        common = [
            str(Path.home() / 'AppData' / 'Local' / 'Programs' / 'MiKTeX' / 'miktex' / 'bin' / 'x64' / (engine_name + '.exe')),
            str(Path('C:/Program Files/MiKTeX/miktex/bin/x64') / (engine_name + '.exe'))
        ]
        for c in common:
            if Path(c).exists():
                return c
        return None

    def build_pdf(self, out: str | Path = None, pandoc_cmd: str | None = None, pdf_engine: str = 'lualatex', header: str | None = None, metadata: str | None = None):
        if pandoc_cmd is None:
            pandoc_cmd = self._find_pandoc()
        # If the pdf_engine is a known name, try to resolve full path so pandoc can call it reliably
        engine_path = None
        if pdf_engine in ('xelatex','lualatex','pdflatex'):
            engine_path = self._find_tex_engine(pdf_engine)
            if engine_path:
                pdf_engine = engine_path
        """Build PDF using pandoc.
        By default use --listings to let LaTeX handle code blocks via the listings package,
        which avoids some issues with verbatim/quote expansion in the LaTeX template.
        """
        out = Path(out or self.formats / 'book.pdf')
        args = [pandoc_cmd]
        # Preprocess manuscript to fence code examples, pass temp files to pandoc
        tmp_dir = self.formats / 'tmp_manuscript'
        srcs = self.collect_markdown()
        tmp_files = self._preprocess_markdown(srcs, tmp_dir)
        args += tmp_files
        args += ['--from', 'markdown+fenced_code_attributes', '--toc', '--pdf-engine', pdf_engine]
        args += ['--syntax-highlighting=pygments']
        if header:
            args += ['--include-in-header', str(header)]
        if metadata:
            args += ['--metadata-file', str(metadata)]
        args += ['-V', 'geometry:margin=1in', '-V', 'lang:vi', '-o', str(out)]
        print('Running:', ' '.join(args))
        subprocess.check_call(args)
        return out

    def _preprocess_markdown(self, src_paths, tmp_dir: Path):
        """Create temporary copies of markdown files with improved auto-fencing and unicode normalization.

        - Normalize unicode to NFC
        - Replace curly quotes and problematic punctuation with ASCII equivalents (optional)
        - Detect runs of code-like lines (>=2 consecutive) and wrap them with ```c fences
        - Preserve existing fenced code blocks
        """
        tmp_dir.mkdir(parents=True, exist_ok=True)
        out_paths = []
        quote_map = {
            '\u201c': '"', '\u201d': '"', '\u2018': "'", '\u2019': "'",
            '\u2013': '-', '\u2014': '-', '\u2212': '-', '\u2192': '->'
        }

        def normalize_line(s: str) -> str:
            s = unicodedata.normalize('NFC', s)
            for k, v in quote_map.items():
                s = s.replace(k, v)
            # collapse CR characters
            s = s.replace('\r', '')
            return s

        code_like_re = re.compile(r"(^\s*#include|\b(int|char|void|float|double|size_t)\b.*\(|\breturn\b|\bprintf\s*\(|\bscanf\s*\(|\bsizeof\b|\bfor\b\s*\(|\bwhile\b\s*\(|\{|\}|;\s*$)")

        for src in src_paths:
            srcp = Path(src)
            outp = tmp_dir / srcp.name
            lines = srcp.read_text(encoding='utf-8').splitlines()
            # first pass: normalize and mark existing fenced blocks
            normalized = []
            in_fence = False
            fence_delim = None
            for raw in lines:
                line = normalize_line(raw)
                if not in_fence and line.lstrip().startswith('```'):
                    in_fence = True
                    fence_delim = line.lstrip()
                    normalized.append(line)
                    continue
                if in_fence:
                    normalized.append(line)
                    if line.lstrip().startswith('```') and line.lstrip() == fence_delim:
                        in_fence = False
                        fence_delim = None
                    continue
                normalized.append(line)
            # second pass: detect runs of code-like lines outside fences
            out_lines = []
            i = 0
            N = len(normalized)
            while i < N:
                line = normalized[i]
                if line.lstrip().startswith('```'):
                    # copy fence block as-is
                    out_lines.append(line)
                    i += 1
                    while i < N:
                        out_lines.append(normalized[i])
                        if normalized[i].lstrip().startswith('```'):
                            i += 1
                            break
                        i += 1
                    continue
                # sanitize common backslash escapes in normal text to avoid LaTeX treating them as commands
                sanitized = line
                sanitized = sanitized.replace('\\n', '`\\n`').replace('\\t', '`\\t`').replace('\\r', '`\\r`').replace('\\0', '`\\0`')
                # compute run length of code-like lines outside fences
                run_start = i
                run_len = 0
                while i < N and not normalized[i].lstrip().startswith('```') and code_like_re.search(normalized[i]):
                    run_len += 1
                    i += 1
                if run_len >= 2:
                    # fence the run from run_start to i-1; keep code-like lines unchanged
                    out_lines.append('```c')
                    out_lines.extend(normalized[run_start:run_start+run_len])
                    out_lines.append('```')
                    # continue; i already at first non-code after run
                    continue
                else:
                    # no fence; write the single sanitized line and advance
                    out_lines.append(sanitized)
                    i = run_start + 1
            # write output
            outp.write_text('\n'.join(out_lines) + '\n', encoding='utf-8')
            out_paths.append(str(outp))
        return out_paths

    def package_code(self, code_dir: str | Path = None, outzip: str | Path = None):
        code_dir = Path(code_dir or (self.root / 'code'))
        outzip = Path(outzip or (self.formats / 'code_samples.zip'))
        if not code_dir.exists():
            raise FileNotFoundError(f'Code directory not found: {code_dir}')
        if outzip.exists():
            outzip.unlink()
        with zipfile.ZipFile(outzip, 'w', compression=zipfile.ZIP_DEFLATED) as z:
            for p in code_dir.rglob('*'):
                if p.is_file():
                    z.write(p, p.relative_to(code_dir))
        return outzip

    def make_kdp_package(self, out: str | Path = None, extra_files: list[str] | None = None):
        out = Path(out or (self.formats / 'kdp_upload_package.zip'))
        files = [self.formats / 'book.epub']
        if (self.formats / 'cover.jpg').exists():
            files.append(self.formats / 'cover.jpg')
        elif (self.formats / 'cover.png').exists():
            files.append(self.formats / 'cover.png')
        if (self.formats / 'code_samples.zip').exists():
            files.append(self.formats / 'code_samples.zip')
        if (self.kdp / 'metadata.yaml').exists():
            files.append(self.kdp / 'metadata.yaml')
        if (self.kdp / 'metadata.json').exists():
            files.append(self.kdp / 'metadata.json')
        if extra_files:
            for e in extra_files:
                files.append(Path(e))
        if (self.formats / 'README_UPLOAD.txt').exists():
            files.append(self.formats / 'README_UPLOAD.txt')
        with zipfile.ZipFile(out, 'w', compression=zipfile.ZIP_DEFLATED) as z:
            for f in files:
                if f.exists():
                    z.write(f, f.name)
        return out

    def generate_simple_cover(self, title: str, author: str, out: str | Path = None, size=(1600, 2560), bg=(18, 85, 160)):
        # Default to JPEG (RGB) — KDP requires RGB, no alpha channel
        out = Path(out or (self.formats / 'cover.jpg'))
        try:
            from PIL import Image, ImageDraw, ImageFont
        except Exception:
            raise RuntimeError('Pillow required for cover generation (pip install pillow)')
        img = Image.new('RGB', size, color=bg)
        draw = ImageDraw.Draw(img)
        # Try system fonts first, fall back to default
        font_paths = [
            ('C:/Windows/Fonts/arialbd.ttf', 'C:/Windows/Fonts/arial.ttf'),
            ('/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf',
             '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf'),
        ]
        font_title = font_author = None
        for bold_path, reg_path in font_paths:
            try:
                font_title  = ImageFont.truetype(bold_path, 90)
                font_author = ImageFont.truetype(reg_path, 48)
                break
            except Exception:
                continue
        if font_title is None:
            font_title = font_author = ImageFont.load_default()
        draw.rectangle([(80, 300), (size[0]-80, 308)], fill=(255, 255, 255))
        bbox = draw.textbbox((0, 0), title, font=font_title)
        tw = bbox[2] - bbox[0]
        draw.text(((size[0] - tw) // 2, 340), title, font=font_title, fill=(255, 255, 255))
        draw.rectangle([(80, size[1]-320), (size[0]-80, size[1]-312)], fill=(255, 255, 255))
        draw.text((100, size[1]-290), f'Tác giả: {author}', font=font_author, fill=(200, 200, 200))
        img.save(out, 'JPEG', quality=95)
        return out


if __name__ == '__main__':
    print('bookgen module. Use the CLI or import BookGenerator.')
