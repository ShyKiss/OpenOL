#!/usr/bin/env python3
"""Tests for Updater.cpp, run under Wine or Proton against real archives and a local HTTP server.

Builds Tests/updater_test.cpp with mingw-w64, so no MSVC is needed.
Usage: test_updater.py [--runner "wine"]  (the runner is the command that starts a .exe)
"""
import argparse
import hashlib
import http.server
import json
import os
import random
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile
import threading
import zipfile
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
failures = []


def check(name, ok, detail=""):
    print(("  ok    " if ok else "  FAIL  ") + name + ("" if ok else f"  [{detail}]"))
    if not ok:
        failures.append(name)


class Harness:
    def __init__(self, exe, runner):
        self.exe, self.runner = exe, runner

    def run(self, *args):
        cmd = self.runner + [self.exe] + [str(a) for a in args]
        env = dict(os.environ, WINEDEBUG="-all")
        p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, env=env, timeout=300)
        return p.returncode, p.stdout.decode("utf-8", "replace").strip()


def winpath(path):
    return "Z:" + path.replace("/", "\\")


def tree(root):
    out = {}
    for d, _, files in os.walk(root):
        for f in files:
            p = os.path.join(d, f)
            with open(p, "rb") as fh:
                out[os.path.relpath(p, root)] = fh.read()
    return out


def new_game(base, name):
    game = os.path.join(base, name)
    shutil.rmtree(game, ignore_errors=True)
    os.makedirs(os.path.join(game, "OpenOL_Update"))
    return game


def payloads():
    rnd = random.Random(1234)
    data = {
        "OutlastLauncher.exe": b"new launcher",
        "Binaries/Win64/OLGame.exe": rnd.randbytes(300000),
        "empty.txt": b"",
        "one.txt": b"x",
        "OLGame/Config/repeat.ini": b"[Section]\nKey=Value\n" * 50000,
        "OLGame/CookedPCConsole/OpenOL/long_matches.u": (b"abc" * 100 + rnd.randbytes(40)) * 3000,
        "OLGame/Localization/RUS/имя файла.txt": "кириллица".encode("utf-8") * 1000,
    }
    return data


def make_zip(path, data, method=zipfile.ZIP_DEFLATED, level=6):
    with zipfile.ZipFile(path, "w", method, compresslevel=level) as z:
        for name, content in data.items():
            z.writestr(name, content)


def test_extract(h, base):
    print("extract")
    data = payloads()
    want = {k.replace("/", os.sep): v for k, v in data.items()}
    for label, method, level in (("deflate-1", zipfile.ZIP_DEFLATED, 1),
                                 ("deflate-9", zipfile.ZIP_DEFLATED, 9),
                                 ("stored", zipfile.ZIP_STORED, 0)):
        game = new_game(base, "ex_" + label)
        zp = os.path.join(base, label + ".zip")
        make_zip(zp, data, method, level)
        rc, out = h.run("extract", winpath(zp), winpath(game))
        got = tree(os.path.join(game, "OpenOL_Update", "staged"))
        check(label, rc == 0 and got == want, out)

    # Fixed Huffman and stored deflate blocks, which zipfile's default level rarely emits.
    for label, strategy, level in (("fixed-blocks", zlib.Z_FIXED, 6), ("stored-blocks", zlib.Z_DEFAULT_STRATEGY, 0)):
        game = new_game(base, "ex_" + label)
        zp = os.path.join(base, label + ".zip")
        rewrite_with_streams(zp, data, strategy, level)
        rc, out = h.run("extract", winpath(zp), winpath(game))
        got = tree(os.path.join(game, "OpenOL_Update", "staged"))
        check(label, rc == 0 and got == want, out)

    # Data descriptors: sizes only in the central directory, as streaming zip writers do it.
    game = new_game(base, "ex_descr")
    zp = os.path.join(base, "descr.zip")
    with open(zp, "wb") as raw:
        class NoSeek:
            def write(self, b): return raw.write(b)
            def flush(self): raw.flush()
            def seekable(self): return False
        with zipfile.ZipFile(NoSeek(), "w", zipfile.ZIP_DEFLATED) as z:
            for name, content in data.items():
                z.writestr(name, content)
    rc, out = h.run("extract", winpath(zp), winpath(game))
    check("data-descriptor", rc == 0 and tree(os.path.join(game, "OpenOL_Update", "staged")) == want, out)


def build_zip(items, strategy=zlib.Z_DEFAULT_STRATEGY, level=6):
    """Writes a zip by hand: exact zlib streams, and entry names that zipfile would normalize."""
    out = bytearray()
    central = bytearray()
    for name, content in items:
        c = zlib.compressobj(level, zlib.DEFLATED, -15, 8, strategy)
        stream = c.compress(content) + c.flush()
        nb = name.encode("utf-8")
        crc = zlib.crc32(content)
        off = len(out)
        out += struct.pack("<IHHHHHIIIHH", 0x04034B50, 20, 0x800, 8, 0, 0, crc, len(stream), len(content), len(nb), 0) + nb + stream
        central += struct.pack("<IHHHHHHIIIHHHHHII", 0x02014B50, 20, 20, 0x800, 8, 0, 0, crc, len(stream),
                               len(content), len(nb), 0, 0, 0, 0, 0, off) + nb
    cd = len(out)
    out += central
    out += struct.pack("<IHHHHIIH", 0x06054B50, 0, 0, len(items), len(items), len(central), cd, 0)
    return bytes(out)


def rewrite_with_streams(path, data, strategy, level):
    with open(path, "wb") as f:
        f.write(build_zip(list(data.items()), strategy, level))


def test_bad_archives(h, base):
    print("bad archives")
    good = os.path.join(base, "good.zip")
    data = payloads()
    make_zip(good, data)
    blob = open(good, "rb").read()

    def expect_fail(label, content, reason, must_not_exist=()):
        game = new_game(base, "bad_" + label)
        zp = os.path.join(base, "bad_" + label + ".zip")
        with open(zp, "wb") as f:
            f.write(content)
        rc, out = h.run("extract", winpath(zp), winpath(game))
        escaped = [p for p in must_not_exist if os.path.exists(os.path.join(base, p))]
        check(label, rc == 1 and reason in out and not escaped, f"rc={rc} {out} escaped={escaped}")

    expect_fail("html-error-page", b"<html>404 Not Found</html>" * 10, "end of central directory")
    expect_fail("empty", b"", "Cannot read")
    expect_fail("truncated", blob[: len(blob) // 2], "end of central directory")

    # Flip a byte inside the largest entry's compressed data.
    with zipfile.ZipFile(good) as z:
        info = max(z.infolist(), key=lambda i: i.compress_size)
    corrupt = bytearray(blob)
    corrupt[info.header_offset + 30 + len(info.filename.encode()) + info.compress_size // 2] ^= 0xFF
    expect_fail("flipped-byte", bytes(corrupt), "Archive is damaged")

    # Valid deflate data, wrong CRC in the central directory.
    badcrc = bytearray(blob)
    cd = blob.rfind(b"PK\x01\x02")
    badcrc[cd + 16] ^= 0xFF
    expect_fail("wrong-crc", bytes(badcrc), "CRC mismatch")

    for label, name in (("slip-dotdot", "../escaped.txt"), ("slip-nested", "OLGame/../../escaped.txt"),
                        ("slip-absolute", "/escaped.txt"), ("slip-drive", "C:/escaped.txt"),
                        ("slip-backslash", "..\\escaped.txt"), ("ads", "file.txt:stream"),
                        ("reserved-name", "NUL.txt"), ("own-workdir", "OpenOL_Update/apply.marker")):
        expect_fail(label, make_named_zip(data, name), "unsafe file name", must_not_exist=("escaped.txt",))

    # Unix symlink entry.
    zp = os.path.join(base, "link.zip")
    make_zip(zp, data)
    with zipfile.ZipFile(zp, "a") as z:
        info = zipfile.ZipInfo("link")
        info.create_system = 3
        info.external_attr = 0o120777 << 16
        z.writestr(info, b"/etc/passwd")
    expect_fail("symlink", open(zp, "rb").read(), "symbolic link")

    # Wrong package: valid archive without the required files.
    zp = os.path.join(base, "other.zip")
    make_zip(zp, {"readme.txt": b"not a release"})
    expect_fail("not-a-release", open(zp, "rb").read(), "required files")

    # Unsupported compression.
    zp = os.path.join(base, "bz.zip")
    make_zip(zp, data, zipfile.ZIP_BZIP2)
    expect_fail("bzip2", open(zp, "rb").read(), "compression method")


def make_named_zip(data, evil_name):
    return build_zip(list(data.items()) + [(evil_name, b"evil")])


def stage(game, files):
    for rel, content in files.items():
        p = os.path.join(game, "OpenOL_Update", "staged", rel)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "wb") as f:
            f.write(content)


def seed(game, files):
    for rel, content in files.items():
        p = os.path.join(game, rel)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "wb") as f:
            f.write(content)


def test_apply(h, base):
    print("apply")
    old = {"a.txt": b"old a", "dir/b.txt": b"old b", "dir/keep.txt": b"untouched"}
    new = {"a.txt": b"new a", "dir/b.txt": b"new b", "dir/added.txt": b"added", "newdir/sub/c.txt": b"c"}
    marker = "apply.marker"

    game = new_game(base, "apply_ok")
    seed(game, old)
    stage(game, new)
    rc, out = h.run("apply", winpath(game))
    live = {k: v for k, v in tree(game).items() if not k.startswith("OpenOL_Update")}
    want = {"a.txt": b"new a", "dir/b.txt": b"new b", "dir/keep.txt": b"untouched",
            "dir/added.txt": b"added", "newdir/sub/c.txt": b"c"}
    check("success", rc == 0 and live == want, f"{out} {live.keys()}")
    check("success-commits", not os.path.exists(os.path.join(game, "OpenOL_Update", marker)))

    # Case differences must resolve to the existing file under Wine as they do on Windows.
    game = new_game(base, "apply_case")
    seed(game, {"OLGame/Localization/OLGame.int": b"old"})
    stage(game, {"olgame/localization/olgame.INT": b"new"})
    rc, out = h.run("apply", winpath(game))
    live = {k: v for k, v in tree(game).items() if not k.startswith("OpenOL_Update")}
    check("case-insensitive", rc == 0 and list(live.values()) == [b"new"] and len(live) == 1, f"{out} {live}")

    # A file held open: everything already replaced must be rolled back.
    game = new_game(base, "apply_locked")
    seed(game, old)
    stage(game, new)
    rc, out = h.run("apply", winpath(game), winpath(os.path.join(game, "dir", "b.txt")))
    live = {k: v for k, v in tree(game).items() if not k.startswith("OpenOL_Update")}
    if rc == 0:
        print("  note  this runner lets a locked file be renamed; rollback path not reached here")
    else:
        check("locked-rolls-back", live == old, f"{out} {live}")
        check("locked-clears-marker", not os.path.exists(os.path.join(game, "OpenOL_Update", marker)))

    # A folder where a file must go: fails after other files were replaced.
    game = new_game(base, "apply_conflict")
    seed(game, old)
    os.makedirs(os.path.join(game, "zz_conflict"))
    stage(game, dict(new, zz_conflict=b"file"))
    rc, out = h.run("apply", winpath(game))
    live = {k: v for k, v in tree(game).items() if not k.startswith("OpenOL_Update")}
    check("conflict-rolls-back", rc == 1 and live == old, f"{rc} {out} {live}")

    # Crash in the middle of the renames: a.txt already new, dir/b.txt moved to backup but
    # its replacement not placed, added.txt new, marker present.
    game = new_game(base, "recover")
    seed(game, {"a.txt": b"new a", "dir/added.txt": b"added", "dir/keep.txt": b"untouched"})
    seed(game, {"OpenOL_Update/backup/a.txt": b"old a", "OpenOL_Update/backup/dir/b.txt": b"old b",
                "OpenOL_Update/" + marker: b"1"})
    rc, out = h.run("recover", winpath(game))
    live = {k: v for k, v in tree(game).items() if not k.startswith("OpenOL_Update")}
    want = {"a.txt": b"old a", "dir/b.txt": b"old b", "dir/keep.txt": b"untouched", "dir/added.txt": b"added"}
    check("crash-recovery", rc == 0 and live == want, f"{out} {live}")
    check("crash-recovery-clears-marker", not os.path.exists(os.path.join(game, "OpenOL_Update", marker)))

    # No marker: the install committed, so a leftover backup must not be restored.
    game = new_game(base, "recover_none")
    seed(game, {"a.txt": b"new a", "OpenOL_Update/backup/a.txt": b"old a"})
    rc, out = h.run("recover", winpath(game))
    check("no-marker-no-restore", rc == 0 and tree(game)["a.txt"] == b"new a", out)

    # Replacing the running exe, as the launcher replaces itself.
    game = new_game(base, "apply_self")
    exe = os.path.join(game, "updater_test.exe")
    shutil.copy(h.exe, exe)
    stage(game, {"updater_test.exe": b"new exe", "other.txt": b"other"})
    self_runner = Harness(exe, h.runner)
    rc, out = self_runner.run("apply-self", winpath(game))
    check("replace-running-exe", rc == 0 and tree(game)["updater_test.exe"] == b"new exe", out)


class Handler(http.server.BaseHTTPRequestHandler):
    body = b""
    flaky = [0]

    def log_message(self, *a):
        pass

    def do_GET(self):
        if self.path == "/ok":
            self.send_response(200); self.send_header("Content-Length", str(len(self.body))); self.end_headers()
            self.wfile.write(self.body)
        elif self.path == "/redirect":
            self.send_response(302); self.send_header("Location", "/ok"); self.send_header("Content-Length", "0"); self.end_headers()
        elif self.path == "/404":
            page = b"<html>Not Found</html>"
            self.send_response(404); self.send_header("Content-Length", str(len(page))); self.end_headers()
            self.wfile.write(page)
        elif self.path == "/truncated":
            self.send_response(200); self.send_header("Content-Length", str(len(self.body))); self.end_headers()
            self.wfile.write(self.body[: len(self.body) // 2])
            self.wfile.flush()
            self.connection.close()
        elif self.path == "/flaky":
            # The first attempt is cut off, the second one succeeds.
            Handler.flaky[0] += 1
            self.send_response(200); self.send_header("Content-Length", str(len(self.body))); self.end_headers()
            if Handler.flaky[0] == 1:
                self.wfile.write(self.body[:1000])
                self.wfile.flush()
                self.connection.close()
            else:
                self.wfile.write(self.body)


def test_download(h, base):
    print("download")
    body = random.Random(5).randbytes(3 * 1024 * 1024)
    Handler.body = body
    srv = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    root = f"http://127.0.0.1:{srv.server_port}"
    sha = hashlib.sha256(body).hexdigest()
    dest = os.path.join(base, "dl.bin")

    def run(path, size=len(body), digest=sha):
        if os.path.exists(dest):
            os.unlink(dest)
        return h.run("download", root + path, size, digest, winpath(dest))

    rc, out = run("/ok")
    check("ok", rc == 0 and open(dest, "rb").read() == body, out)
    rc, out = run("/redirect")
    check("redirect", rc == 0 and open(dest, "rb").read() == body, out)
    rc, out = run("/flaky")
    check("retry-after-cut", rc == 0 and open(dest, "rb").read() == body, out)
    rc, out = run("/404")
    check("http-404", rc == 1 and "404" in out, out)
    rc, out = run("/truncated")
    check("truncated", rc == 1, out)
    rc, out = run("/ok", digest="0" * 64)
    check("wrong-sha256", rc == 1 and "SHA-256" in out, out)
    rc, out = run("/ok", size=len(body) + 1)
    check("wrong-size", rc == 1, out)
    srv.shutdown()


def asset(name, **kw):
    a = {"url": "https://api.github.com/x", "id": 1, "name": name, "label": None,
         "uploader": {"login": "someone", "name": "OpenOL.zip"}, "state": "uploaded", "size": 123,
         "digest": "sha256:" + "ab" * 32,
         "browser_download_url": f"https://github.com/ShyKiss/OpenOL/releases/download/9.9.9/{name}"}
    a.update(kw)
    return a


def test_parse(h, base):
    print("release metadata")
    p = os.path.join(base, "release.json")

    def run(obj, raw=None):
        with open(p, "wb") as f:
            f.write(raw if raw is not None else json.dumps(obj).encode())
        return h.run("parse", winpath(p))

    # Text that imitates fields must not confuse the parser, wherever it appears.
    trap = 'x"tag_name":"99.0.0","browser_download_url":"https://evil.example/OpenOL.zip"'
    rel = {"tag_name": "9.9.9", "name": trap, "author": {"tag_name": "88.0.0"},
           "assets": [asset("OpenOL.7z", size=1), asset("OpenOL.zip.sig", size=2), asset("OpenOL.zip")],
           "body": trap * 20000}
    want = "9.9.9 123 https://github.com/ShyKiss/OpenOL/releases/download/9.9.9/OpenOL.zip " + "ab" * 32
    rc, out = run(rel)
    check("picks-exact-asset", rc == 0 and out.startswith(want), out)
    rc, out = run(rel, json.dumps(rel, indent=2).encode())
    check("pretty-printed", rc == 0 and out.startswith(want), out)

    def expect_fail(label, **patch):
        r = dict(rel, assets=[asset("OpenOL.zip", **patch)])
        rc, out = run(r)
        check(label, rc == 1, out)

    expect_fail("foreign-url", browser_download_url="https://evil.example/OpenOL.zip")
    expect_fail("no-digest", digest=None)
    expect_fail("bad-digest", digest="sha256:zz")
    expect_fail("md5-digest", digest="md5:" + "ab" * 16)
    expect_fail("still-uploading", state="starter")
    expect_fail("zero-size", size=0)
    rc, out = run(dict(rel, assets=[asset("OpenOL.7z")]))
    check("no-zip-asset", rc == 1, out)
    rc, out = run(None, b'{"message":"API rate limit exceeded"}')
    check("rate-limit-reply", rc == 1, out)
    rc, out = run(None, json.dumps(rel).encode()[:200])
    check("cut-off-json", rc == 1, out)

    print("versions")
    for a, b, want_newer in (("2.1.1", "2.1.0", True), ("v2.2.0", "2.1.9", True), ("3.0.0", "2.9.9", True),
                             ("2.1.0", "2.1.0", False), ("2.0.9", "2.1.0", False), ("2.10.0", "2.9.0", True),
                             ("garbage", "2.1.0", False)):
        rc, _ = h.run("newer", a, b)
        check(f"{a} > {b} is {want_newer}", (rc == 0) == want_newer)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--runner", default="wine")
    args = ap.parse_args()

    base = tempfile.mkdtemp(prefix="updater_test_", dir=os.environ.get("TMPDIR"))
    try:
        exe = os.path.join(base, "updater_test.exe")
        # mingw names this header in lower case, the Windows SDK does not.
        with open(os.path.join(base, "SDKDDKVer.h"), "w") as f:
            f.write("#include <sdkddkver.h>\n")
        subprocess.check_call(["i686-w64-mingw32-g++", "-O2", "-std=gnu++11", "-municode", "-DUNICODE", "-D_UNICODE", "-static",
                               "-I" + base, "-Wno-write-strings", os.path.join(HERE, "updater_test.cpp"), "-o", exe,
                               "-lwininet", "-lshlwapi", "-ladvapi32", "-luser32", "-lgdi32"])
        h = Harness(exe, shlex.split(args.runner))
        test_parse(h, base)
        test_extract(h, base)
        test_bad_archives(h, base)
        test_download(h, base)
        test_apply(h, base)
    finally:
        shutil.rmtree(base, ignore_errors=True)

    print(f"\n{len(failures)} failed" if failures else "\nall passed")
    if failures:
        print("\n".join("  " + f for f in failures))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
