#!/usr/bin/env python3
"""Minimal Android boot image (header v3/v4) unpack/pack.

Layout (v3/v4, page 4096):
  [0, header_size)        header
  page-aligned kernel
  page-aligned ramdisk
  optional trailing data (bootconfig / AVB signature / padding)

We never rebuild the header from scratch: the original header bytes are kept
verbatim and only ramdisk_size (offset 12) and optionally cmdline are patched.
That keeps unknown/reserved fields (e.g. the OnePlus 1584-byte header) intact.
"""
import argparse
import json
import os
import struct
import sys

PAGE = 4096


def align(x, a=PAGE):
    return (x + a - 1) // a * a


def read_header(f):
    f.seek(0)
    head = f.read(4096)
    if head[:8] != b"ANDROID!":
        raise SystemExit("not an Android boot image (magic %r)" % head[:8])
    kernel_size, ramdisk_size, os_version, header_size = struct.unpack_from("<4I", head, 8)
    header_version = struct.unpack_from("<I", head, 40)[0]
    cmdline = head[44:44 + 1536].split(b"\0")[0].decode("utf-8", "replace")
    return {
        "kernel_size": kernel_size,
        "ramdisk_size": ramdisk_size,
        "os_version": os_version,
        "header_size": header_size,
        "header_version": header_version,
        "cmdline": cmdline,
        "raw_header": head[:header_size],
    }


def unpack(img, outdir):
    os.makedirs(outdir, exist_ok=True)
    with open(img, "rb") as f:
        h = read_header(f)
        total = os.fstat(f.fileno()).st_size
        koff = PAGE
        roff = align(koff + h["kernel_size"])
        toff = align(roff + h["ramdisk_size"])
        f.seek(koff)
        kernel = f.read(h["kernel_size"])
        f.seek(roff)
        ramdisk = f.read(h["ramdisk_size"])
        f.seek(toff)
        tail = f.read(total - toff)
    open(os.path.join(outdir, "kernel"), "wb").write(kernel)
    open(os.path.join(outdir, "ramdisk.gz"), "wb").write(ramdisk)
    with open(os.path.join(outdir, "tail.bin"), "wb") as t:
        t.write(tail)
    meta = {k: v for k, v in h.items() if k != "raw_header"}
    meta.update({"total_size": total, "kernel_off": koff, "ramdisk_off": roff,
                 "tail_off": toff, "tail_len": len(tail)})
    json.dump(meta, open(os.path.join(outdir, "meta.json"), "w"), indent=2)
    print(json.dumps(meta, indent=2))
    print("kernel   %d -> %s" % (len(kernel), os.path.join(outdir, "kernel")))
    print("ramdisk  %d -> %s" % (len(ramdisk), os.path.join(outdir, "ramdisk.gz")))
    print("tail     %d -> %s" % (len(tail), os.path.join(outdir, "tail.bin")))


def pack(outdir, img, ramdisk=None, cmdline=None, keep_tail=False):
    meta = json.load(open(os.path.join(outdir, "meta.json")))
    if meta["header_version"] < 3:
        raise SystemExit("only v3/v4 supported (this is v%d)" % meta["header_version"])
    header_size = meta["header_size"]
    head = bytearray(open(os.path.join(outdir, "header.bin"), "rb").read()
                     if os.path.exists(os.path.join(outdir, "header.bin"))
                     else b"")
    if not head:
        # rebuild the raw header from the source image recorded in meta
        src = meta.get("_source")
        if not src:
            raise SystemExit("header.bin missing and meta has no _source")
        with open(src, "rb") as f:
            head = bytearray(f.read(header_size))
    kernel = open(os.path.join(outdir, "kernel"), "rb").read()
    rd_path = ramdisk or os.path.join(outdir, "ramdisk.gz")
    rd = open(rd_path, "rb").read()
    tail = open(os.path.join(outdir, "tail.bin"), "rb").read() if keep_tail else b""
    struct.pack_into("<I", head, 12, len(rd))
    if cmdline is not None:
        c = cmdline.encode() + b"\0"
        if len(c) > 1536:
            raise SystemExit("cmdline too long")
        head[44:44 + 1536] = c + b"\0" * (1536 - len(c))
    out = bytearray()
    out += head[:header_size]
    out += b"\0" * (PAGE - len(out))
    out += kernel
    out += b"\0" * (align(len(out)) - len(out))
    out += rd
    out += b"\0" * (align(len(out)) - len(out))
    out += tail
    with open(img, "wb") as f:
        f.write(out)
    print("packed %s: header=%d kernel=%d ramdisk=%d tail=%d total=%d"
          % (img, header_size, len(kernel), len(rd), len(tail), len(out)))


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    u = sub.add_parser("unpack")
    u.add_argument("img")
    u.add_argument("outdir")
    p = sub.add_parser("pack")
    p.add_argument("outdir")
    p.add_argument("img")
    p.add_argument("--ramdisk")
    p.add_argument("--cmdline")
    p.add_argument("--keep-tail", action="store_true")
    a = ap.parse_args()
    if a.cmd == "unpack":
        unpack(a.img, a.outdir)
        meta_path = os.path.join(a.outdir, "meta.json")
        meta = json.load(open(meta_path))
        meta["_source"] = os.path.abspath(a.img)
        json.dump(meta, open(meta_path, "w"), indent=2)
    else:
        pack(a.outdir, a.img, a.ramdisk, a.cmdline, a.keep_tail)


if __name__ == "__main__":
    main()
