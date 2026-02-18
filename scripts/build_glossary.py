#!/usr/bin/env python3
"""
Build or update data/glossary.txt from a word list using:
  1. WordNet (NLTK) — primary
  2. Wiktionary API — fill gaps
  3. LLM (Ollama or OpenAI) — optional, for remaining gaps

Supports --resume (continue from last checkpoint), --retries, and per-step timeouts.
On timeout or failure after retries, a checkpoint is saved; run with --resume to continue.

Usage:
  python build_glossary.py [word_list.txt] [--output path] [--no-wiktionary] [--llm]
  python build_glossary.py --resume [--wiktionary-timeout 3600] ...
  LLM gap-fill is off by default; use --llm to enable.
  If word_list.txt omitted, uses ../../data/dict/en_US.dic (relative to script).
  Output defaults to ../../data/glossary.txt.

Requires: pip install -r scripts/requirements-glossary.txt
          python -c "import nltk; nltk.download('wordnet'); nltk.download('omw-1.4')"
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import tempfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
DEFAULT_DIC = os.path.join(PROJECT_ROOT, "data", "dict", "en_US.dic")
DEFAULT_GLOSSARY = os.path.join(PROJECT_ROOT, "data", "glossary.txt")

# Default timeouts (seconds)
DEFAULT_WORDNET_TIMEOUT = 300
DEFAULT_WIKTIONARY_TIMEOUT = 600
DEFAULT_LLM_TIMEOUT = 300
DEFAULT_RETRIES = 3


def read_word_list(path: str) -> set[str]:
    words = set()
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if line.isdigit():
                continue
            w = line.split("/")[0].strip()
            if w:
                words.add(w.lower())
    return words


def read_glossary_lines(path: str) -> dict[str, list[tuple[str, str]]]:
    """Return dict: word -> [(pos, gloss), ...]"""
    out = {}
    if not os.path.exists(path):
        return out
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or "\t" not in line:
                continue
            parts = line.split("\t", 2)
            if len(parts) < 3:
                continue
            w, pos, gloss = parts[0].strip().lower(), parts[1].strip(), parts[2].strip()
            if not w or not gloss:
                continue
            out.setdefault(w, []).append((pos, gloss))
    return out


def resume_dir_for(output_path: str) -> str:
    """Directory for checkpoint files (merged.tsv, missing.txt)."""
    return output_path + ".resume"


def save_checkpoint(resume_dir: str, merged: dict, missing: set[str]) -> None:
    os.makedirs(resume_dir, exist_ok=True)
    merged_path = os.path.join(resume_dir, "merged.tsv")
    with open(merged_path, "w", encoding="utf-8") as f:
        for w in sorted(merged):
            for pos, gloss in merged[w]:
                f.write(f"{w}\t{pos}\t{gloss}\n")
    missing_path = os.path.join(resume_dir, "missing.txt")
    with open(missing_path, "w", encoding="utf-8") as f:
        for w in sorted(missing):
            f.write(w + "\n")


def load_checkpoint(resume_dir: str) -> tuple[dict[str, list[tuple[str, str]]], set[str]] | None:
    merged_path = os.path.join(resume_dir, "merged.tsv")
    missing_path = os.path.join(resume_dir, "missing.txt")
    if not os.path.exists(merged_path) or not os.path.exists(missing_path):
        return None
    merged = read_glossary_lines(merged_path)
    missing = set()
    with open(missing_path, "r", encoding="utf-8") as f:
        for line in f:
            w = line.strip().lower()
            if w:
                missing.add(w)
    return merged, missing


def run_wordnet(words: set[str], timeout: int, retries: int) -> dict[str, list[tuple[str, str]]] | None:
    with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
        for w in sorted(words):
            f.write(w + "\n")
        tmp = f.name
    try:
        for attempt in range(1, retries + 1):
            try:
                out = subprocess.run(
                    [sys.executable, os.path.join(SCRIPT_DIR, "glossary_wordnet.py"), tmp],
                    capture_output=True,
                    text=True,
                    cwd=SCRIPT_DIR,
                    timeout=timeout,
                )
                if out.returncode != 0 and attempt < retries:
                    print(f"  WordNet attempt {attempt}/{retries} failed (exit {out.returncode}), retrying...", file=sys.stderr)
                    continue
                result = {}
                for line in (out.stdout or "").splitlines():
                    if "\t" not in line:
                        continue
                    parts = line.split("\t", 2)
                    if len(parts) < 3:
                        continue
                    w, pos, gloss = parts[0].strip().lower(), parts[1].strip(), parts[2].strip()
                    if w and gloss:
                        result.setdefault(w, []).append((pos, gloss))
                return result
            except subprocess.TimeoutExpired:
                if attempt < retries:
                    print(f"  WordNet attempt {attempt}/{retries} timed out after {timeout}s, retrying...", file=sys.stderr)
                else:
                    print(f"  WordNet timed out after {retries} attempts.", file=sys.stderr)
                    return None
        return None
    finally:
        os.unlink(tmp)


def run_wiktionary(words: set[str], timeout: int, retries: int) -> dict[str, list[tuple[str, str]]] | None:
    with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
        for w in sorted(words):
            f.write(w + "\n")
        tmp = f.name
    try:
        for attempt in range(1, retries + 1):
            try:
                out = subprocess.run(
                    [sys.executable, os.path.join(SCRIPT_DIR, "glossary_wiktionary.py"), tmp],
                    capture_output=True,
                    text=True,
                    cwd=SCRIPT_DIR,
                    timeout=timeout,
                )
                if out.returncode != 0 and attempt < retries:
                    print(f"  Wiktionary attempt {attempt}/{retries} failed (exit {out.returncode}), retrying...", file=sys.stderr)
                    continue
                result = {}
                for line in (out.stdout or "").splitlines():
                    if "\t" not in line or line.startswith("#"):
                        continue
                    parts = line.split("\t", 2)
                    if len(parts) < 3:
                        continue
                    w, pos, gloss = parts[0].strip().lower(), parts[1].strip(), parts[2].strip()
                    if w and gloss:
                        result.setdefault(w, []).append((pos, gloss))
                return result
            except subprocess.TimeoutExpired:
                if attempt < retries:
                    print(f"  Wiktionary attempt {attempt}/{retries} timed out after {timeout}s, retrying...", file=sys.stderr)
                else:
                    print(f"  Wiktionary timed out after {retries} attempts. Use --resume to continue later.", file=sys.stderr)
                    return None
        return None
    finally:
        os.unlink(tmp)


def run_llm(words: set[str], timeout: int, retries: int) -> dict[str, list[tuple[str, str]]] | None:
    with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
        for w in sorted(words):
            f.write(w + "\n")
        tmp = f.name
    try:
        for attempt in range(1, retries + 1):
            try:
                out = subprocess.run(
                    [sys.executable, os.path.join(SCRIPT_DIR, "glossary_llm.py"), tmp],
                    capture_output=True,
                    text=True,
                    cwd=SCRIPT_DIR,
                    timeout=timeout,
                    env={**os.environ},
                )
                if out.returncode != 0 and attempt < retries:
                    print(f"  LLM attempt {attempt}/{retries} failed (exit {out.returncode}), retrying...", file=sys.stderr)
                    continue
                result = {}
                for line in (out.stdout or "").splitlines():
                    if "\t" not in line:
                        continue
                    parts = line.split("\t", 2)
                    if len(parts) < 3:
                        continue
                    w, pos, gloss = parts[0].strip().lower(), parts[1].strip(), parts[2].strip()
                    if w and gloss:
                        result.setdefault(w, []).append((pos, gloss))
                return result
            except subprocess.TimeoutExpired:
                if attempt < retries:
                    print(f"  LLM attempt {attempt}/{retries} timed out after {timeout}s, retrying...", file=sys.stderr)
                else:
                    print(f"  LLM timed out after {retries} attempts. Use --resume to continue later.", file=sys.stderr)
                    return None
        return None
    finally:
        os.unlink(tmp)


def main():
    ap = argparse.ArgumentParser(description="Build glossary from word list (WordNet → Wiktionary → LLM)")
    ap.add_argument("word_list", nargs="?", default=DEFAULT_DIC, help="Path to word list or .dic file")
    ap.add_argument("-o", "--output", default=DEFAULT_GLOSSARY, help="Output glossary path")
    ap.add_argument("--no-wiktionary", action="store_true", help="Skip Wiktionary gap-fill")
    ap.add_argument("--llm", action="store_true", help="Enable LLM gap-fill (off by default)")
    ap.add_argument("--resume", action="store_true", help="Load checkpoint and continue from last run")
    ap.add_argument("--retries", type=int, default=DEFAULT_RETRIES, help=f"Retries per step on timeout/failure (default {DEFAULT_RETRIES})")
    ap.add_argument("--wordnet-timeout", type=int, default=DEFAULT_WORDNET_TIMEOUT, help=f"WordNet step timeout in seconds (default {DEFAULT_WORDNET_TIMEOUT})")
    ap.add_argument("--wiktionary-timeout", type=int, default=DEFAULT_WIKTIONARY_TIMEOUT, help=f"Wiktionary step timeout in seconds (default {DEFAULT_WIKTIONARY_TIMEOUT})")
    ap.add_argument("--llm-timeout", type=int, default=DEFAULT_LLM_TIMEOUT, help=f"LLM step timeout in seconds (default {DEFAULT_LLM_TIMEOUT})")
    args = ap.parse_args()

    resume_dir = resume_dir_for(args.output)
    merged: dict[str, list[tuple[str, str]]] = {}
    missing: set[str] = set()

    if args.resume:
        loaded = load_checkpoint(resume_dir)
        if loaded:
            merged, missing = loaded
            print(f"Resumed: {len(merged)} defined, {len(missing)} still missing", file=sys.stderr)
        else:
            print("No checkpoint found; starting from scratch.", file=sys.stderr)

    if not args.resume or not merged:
        if not os.path.exists(args.word_list):
            print(f"Word list not found: {args.word_list}", file=sys.stderr)
            sys.exit(1)
        words = read_word_list(args.word_list)
        print(f"Words to define: {len(words)}", file=sys.stderr)
        # 1. WordNet
        print("Step 1: WordNet...", file=sys.stderr)
        wn = run_wordnet(words, timeout=args.wordnet_timeout, retries=args.retries)
        if wn is None:
            print("WordNet failed after retries. Checkpoint not saved (no progress).", file=sys.stderr)
            sys.exit(1)
        for w, entries in wn.items():
            merged[w] = entries
        missing = words - set(merged)
        print(f"  Defined: {len(merged)}, missing: {len(missing)}", file=sys.stderr)
        save_checkpoint(resume_dir, merged, missing)
    else:
        words = set(merged) | missing

    # 2. Wiktionary
    if missing and not args.no_wiktionary:
        print("Step 2: Wiktionary (gaps)...", file=sys.stderr)
        wk = run_wiktionary(missing, timeout=args.wiktionary_timeout, retries=args.retries)
        if wk is None:
            save_checkpoint(resume_dir, merged, missing)
            print(f"Checkpoint saved. Resume with: python build_glossary.py --resume -o {args.output}", file=sys.stderr)
            sys.exit(1)
        for w, entries in wk.items():
            merged[w] = entries
        missing = words - set(merged)
        print(f"  Defined: {len(merged)}, missing: {len(missing)}", file=sys.stderr)
        save_checkpoint(resume_dir, merged, missing)

    # 3. LLM (optional; off by default)
    if missing and args.llm:
        print("Step 3: LLM (gaps)...", file=sys.stderr)
        llm = run_llm(missing, timeout=args.llm_timeout, retries=args.retries)
        if llm is None:
            save_checkpoint(resume_dir, merged, missing)
            print(f"Checkpoint saved. Resume with: python build_glossary.py --resume -o {args.output}", file=sys.stderr)
            sys.exit(1)
        for w, entries in llm.items():
            merged[w] = entries
        missing = words - set(merged)
        print(f"  Defined: {len(merged)}, missing: {len(missing)}", file=sys.stderr)
        save_checkpoint(resume_dir, merged, missing)

    # Success: write final glossary and remove checkpoint
    os.makedirs(os.path.dirname(args.output) or ".", exist_ok=True)
    with open(args.output, "w", encoding="utf-8") as f:
        f.write("# spell glossary: word<TAB>pos<TAB>short definition\n")
        f.write("# Built by scripts/build_glossary.py (WordNet → Wiktionary → LLM)\n")
        f.write("# Lines starting with # are comments.\n\n")
        for w in sorted(merged):
            for pos, gloss in merged[w]:
                f.write(f"{w}\t{pos}\t{gloss}\n")

    if os.path.isdir(resume_dir):
        shutil.rmtree(resume_dir)
        print("Checkpoint cleared.", file=sys.stderr)

    print(f"Wrote {args.output} ({len(merged)} words, {sum(len(v) for v in merged.values())} entries)", file=sys.stderr)
    if missing:
        print(f"Still missing: {len(missing)} words", file=sys.stderr)


if __name__ == "__main__":
    main()
