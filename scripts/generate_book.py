"""Entry point: python scripts/generate_book.py <action> [options]"""
import sys
import subprocess
from pathlib import Path

repo = Path(__file__).resolve().parents[1]
args = [sys.executable, '-m', 'tools.bookgen.cli'] + sys.argv[1:]
subprocess.run(args, cwd=str(repo))
