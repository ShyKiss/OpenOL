#!/usr/bin/env python3
"""Find source files that git cannot see because of the default-deny .gitignore.

The repository tracks only files that differ from the original UE3 tree. An edit
to an engine file that is not tracked yet is invisible to `git status`, so this
script compares the working tree with the original and reports the difference.

Only files modified since the last clean scan are compared, so a scan takes
seconds even on a cold disk cache.

    git_track.py            list changed/new untracked files
    git_track.py --add      stage them with `git add -f`
    git_track.py --check    exit 1 if any exist
    git_track.py --full     ignore the timestamp and compare every file
"""
import argparse
import os
import subprocess
import sys
import time

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
BASE = os.path.join(os.path.dirname(ROOT), "Development")
OURS = os.path.join(ROOT, "Development")
STAMP = os.path.join(ROOT, ".git", "git_track_stamp")
# Safety margin for coarse timestamps and files saved during the previous scan.
STAMP_MARGIN = 5.0

SOURCE_EXT = {".cpp", ".c", ".h", ".hpp", ".inl", ".uc", ".cs", ".csproj", ".vcxproj", ".sln"}
SCRIPT_EXT = {".sh", ".py"}

EXCLUDED = {
    "Src": ["MyMod", "Picker", "DEL_Picker", "Multiplayer/server", "Multiplayer/server121111",
            "Multiplayer/ClassesBak", "Multiplayer/IncBak", "Multiplayer/SrcBak",
            "UnrealBuildTool/obj", "Targets/clangd_setup"],
    "Tools": ["CookerSync/obj"],
    "External": ["Steamworks/sdk_new", "GFx-4.2"],
}
EXCLUDED_FILES = {"Development/Src/Multiplayer/server.py"}
SKIP_DIR_NAMES = {"__pycache__", ".git", ".clangd-msvc-shim", "obj", "Intermediate"}


def same_content(a, b):
    if os.path.getsize(a) != os.path.getsize(b):
        return False
    with open(a, "rb") as fa, open(b, "rb") as fb:
        while True:
            ca, cb = fa.read(1 << 20), fb.read(1 << 20)
            if ca != cb:
                return False
            if not ca:
                return True


def tracked_files():
    out = subprocess.check_output(["git", "-C", ROOT, "ls-files", "-z", "--", "Development"])
    return {p.decode("utf-8") for p in out.split(b"\0") if p}


def scan(sub, tracked, since):
    top = os.path.join(OURS, sub)
    excluded = {os.path.join(top, d) for d in EXCLUDED[sub]}
    for dirpath, dirnames, filenames in os.walk(top):
        dirnames[:] = [d for d in dirnames
                       if d not in SKIP_DIR_NAMES and os.path.join(dirpath, d) not in excluded]
        for name in filenames:
            ext = os.path.splitext(name)[1].lower()
            if ext not in SOURCE_EXT and ext not in SCRIPT_EXT:
                continue
            path = os.path.join(dirpath, name)
            if os.path.getmtime(path) < since:
                continue
            rel_root = os.path.relpath(path, ROOT).replace(os.sep, "/")
            if rel_root in tracked or rel_root in EXCLUDED_FILES:
                continue
            original = os.path.join(BASE, os.path.relpath(path, OURS))
            if not os.path.isfile(original):
                yield "new", rel_root
            elif not same_content(path, original):
                yield "changed", rel_root


def read_stamp():
    try:
        with open(STAMP) as f:
            return float(f.read()) - STAMP_MARGIN
    except (OSError, ValueError):
        return 0.0


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--add", action="store_true", help="stage the files with git add -f")
    parser.add_argument("--check", action="store_true", help="exit 1 if any files are found")
    parser.add_argument("--full", action="store_true", help="compare every file")
    args = parser.parse_args()

    if not os.path.isdir(BASE):
        sys.exit("Original tree not found: " + BASE)

    started = time.time()
    since = 0.0 if args.full else read_stamp()
    tracked = tracked_files()
    found = sorted((path, kind) for sub in EXCLUDED for kind, path in scan(sub, tracked, since))
    for path, kind in found:
        print("%-8s%s" % (kind, path))

    if args.add and found:
        subprocess.check_call(["git", "-C", ROOT, "add", "-f", "--"] + [p for p, _ in found])
    # Unhandled files must be reported again next time, so keep the old stamp for them.
    if not found or args.add:
        with open(STAMP, "w") as f:
            f.write(str(started))
    if args.check and found:
        sys.exit(1)


if __name__ == "__main__":
    main()
