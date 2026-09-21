"""Simple CLI for the book generator."""
from argparse import ArgumentParser
from .generator import BookGenerator
from pathlib import Path

def main():
    p = ArgumentParser()
    p.add_argument('--repo', default='.', help='Repo root')
    p.add_argument('action', choices=['epub','pdf','cover','package','codezip'])
    p.add_argument('--output', help='Output path')
    p.add_argument('--title', help='Title for cover')
    p.add_argument('--author', help='Author for cover')
    args = p.parse_args()
    gen = BookGenerator(args.repo)
    if args.action == 'epub':
        out = gen.build_epub(out=args.output or (Path(args.repo)/'formats'/'book.epub'), css=Path(args.repo)/'formats'/'styles.css', metadata=Path(args.repo)/'kdp'/'metadata.yaml')
        print('EPUB built:', out)
    elif args.action == 'pdf':
        out = gen.build_pdf(out=args.output or (Path(args.repo)/'formats'/'book.pdf'), header=Path(args.repo)/'formats'/'pdf_header.tex', metadata=Path(args.repo)/'kdp'/'metadata.yaml')
        print('PDF built:', out)
    elif args.action == 'cover':
        title = args.title or 'Học Lập Trình C - Từ Cơ Bản đến Nâng Cao'
        author = args.author or 'Lukas'
        out = gen.generate_simple_cover(title, author, out=args.output or (Path(args.repo)/'formats'/'cover.png'))
        print('Cover written:', out)
    elif args.action == 'codezip':
        out = gen.package_code(code_dir=Path(args.repo)/'code', outzip=args.output or (Path(args.repo)/'formats'/'code_samples.zip'))
        print('Code zip:', out)
    elif args.action == 'package':
        out = gen.make_kdp_package(out=args.output or (Path(args.repo)/'formats'/'kdp_upload_package.zip'))
        print('KDP package:', out)

if __name__ == '__main__':
    main()
