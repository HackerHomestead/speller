#!/usr/bin/env python3
"""
Update the bundled dictionary (optional) and rebuild the glossary.

Usage:
  python update_dict_glossary.py [--fetch-dict] [--no-wiktionary] [--llm]
                                 [--resume] [--retries N] [--wiktionary-timeout SEC] ...

  --fetch-dict   Download en_US.aff and en_US.dic from LibreOffice dictionaries
                 into data/dict/ (backup existing first). Then rebuild glossary
                 from the new word list.
  --llm          Enable LLM gap-fill (off by default; use for remaining words after WordNet/Wiktionary).
  --resume       Continue from last checkpoint (after timeout/failure).
  --retries N    Retries per step (default 3).
  --wiktionary-timeout SEC   Timeout for Wiktionary step (default 600).
  Without --fetch-dict, only rebuilds data/glossary.txt from existing data/dict/en_US.dic.

Requires: pip install -r scripts/requirements-glossary.txt (for glossary build)
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import urllib.error
import urllib.request

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
DICT_DIR = os.path.join(PROJECT_ROOT, "data", "dict")
# LibreOffice English US dictionary (repo uses branch "master", not "main")
LO_EN_AFF = "https://raw.githubusercontent.com/LibreOffice/dictionaries/master/en/en_US.aff"
LO_EN_DIC = "https://raw.githubusercontent.com/LibreOffice/dictionaries/master/en/en_US.dic"


def fetch_url(url: str, path: str) -> bool:
    """Fetch URL to path. Return True on success, False on HTTP error."""
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "speller-update-script/1.0"})
        with urllib.request.urlopen(req, timeout=30) as resp:
            if resp.status != 200:
                print(f"[update_dict_glossary] ERROR: {url} returned HTTP {resp.status}", file=sys.stderr)
                return False
            with open(path, "wb") as f:
                f.write(resp.read())
        return True
    except urllib.error.HTTPError as e:
        print(f"[update_dict_glossary] ERROR: {url} -> HTTP {e.code} {e.reason}", file=sys.stderr)
        return False
    except OSError as e:
        print(f"[update_dict_glossary] ERROR: {url} -> {e}", file=sys.stderr)
        return False


def fetch_dict() -> bool:
    os.makedirs(DICT_DIR, exist_ok=True)
    aff_path = os.path.join(DICT_DIR, "en_US.aff")
    dic_path = os.path.join(DICT_DIR, "en_US.dic")
    print("[update_dict_glossary] Dictionary directory:", DICT_DIR, file=sys.stderr)
    for path in (aff_path, dic_path):
        if os.path.exists(path):
            backup = path + ".bak"
            shutil.copy2(path, backup)
            print(f"[update_dict_glossary] Backed up {path} -> {backup}", file=sys.stderr)
    print("[update_dict_glossary] Fetching en_US.aff from LibreOffice dictionaries (master branch)...", file=sys.stderr)
    if not fetch_url(LO_EN_AFF, aff_path):
        return False
    print("[update_dict_glossary] Fetching en_US.dic...", file=sys.stderr)
    if not fetch_url(LO_EN_DIC, dic_path):
        return False
    print("[update_dict_glossary] Dictionary files updated.", file=sys.stderr)
    return True


def main():
    ap = argparse.ArgumentParser(description="Update dictionary (optional) and rebuild glossary")
    ap.add_argument("--fetch-dict", action="store_true", help="Download en_US from LibreOffice dicts")
    ap.add_argument("--no-wiktionary", action="store_true", help="Skip Wiktionary in glossary build")
    ap.add_argument("--llm", action="store_true", help="Enable LLM gap-fill (off by default)")
    ap.add_argument("--resume", action="store_true", help="Resume glossary build from last checkpoint")
    ap.add_argument("--retries", type=int, default=3, help="Retries per step (default 3)")
    ap.add_argument("--wiktionary-timeout", type=int, default=600, help="Wiktionary step timeout in seconds (default 600)")
    ap.add_argument("--llm-timeout", type=int, default=300, help="LLM step timeout in seconds (default 300)")
    ap.add_argument("--wordnet-timeout", type=int, default=300, help="WordNet step timeout in seconds (default 300)")
    args = ap.parse_args()

    word_list = os.path.join(PROJECT_ROOT, "data", "dict", "en_US.dic")
    output_glossary = os.path.join(PROJECT_ROOT, "data", "glossary.txt")

    if args.fetch_dict:
        print("[update_dict_glossary] Fetching dictionary first...", file=sys.stderr)
        if not fetch_dict():
            print("[update_dict_glossary] Dictionary fetch failed. Check URLs (LibreOffice uses branch 'master').", file=sys.stderr)
            sys.exit(1)

    print("[update_dict_glossary] Word list:", word_list, file=sys.stderr)
    if not os.path.exists(word_list):
        print("[update_dict_glossary] ERROR: Word list not found. Use --fetch-dict to download dictionary first.", file=sys.stderr)
        sys.exit(1)

    print("[update_dict_glossary] Running build_glossary.py (WordNet -> Wiktionary -> LLM)...", file=sys.stderr)
    print("[update_dict_glossary] Output glossary:", output_glossary, file=sys.stderr)
    build = os.path.join(SCRIPT_DIR, "build_glossary.py")
    cmd = [sys.executable, build, word_list, "-o", output_glossary]
    if args.no_wiktionary:
        cmd.append("--no-wiktionary")
        print("[update_dict_glossary] Wiktionary disabled (--no-wiktionary).", file=sys.stderr)
    if args.llm:
        cmd.append("--llm")
        print("[update_dict_glossary] LLM gap-fill enabled (--llm).", file=sys.stderr)
    else:
        print("[update_dict_glossary] LLM gap-fill is disabled by default; use --llm to enable.", file=sys.stderr)
    if args.resume:
        cmd.append("--resume")
        print("[update_dict_glossary] Resuming from checkpoint.", file=sys.stderr)
    cmd.extend(["--retries", str(args.retries)])
    cmd.extend(["--wordnet-timeout", str(args.wordnet_timeout)])
    cmd.extend(["--wiktionary-timeout", str(args.wiktionary_timeout)])
    cmd.extend(["--llm-timeout", str(args.llm_timeout)])
    r = subprocess.run(cmd, cwd=PROJECT_ROOT)
    if r.returncode != 0:
        print("[update_dict_glossary] build_glossary.py exited with code", r.returncode, file=sys.stderr)
        sys.exit(r.returncode)
    print("[update_dict_glossary] Glossary rebuild done.", file=sys.stderr)


if __name__ == "__main__":
    main()
