#!/usr/bin/env python3
"""Build an Android sparse image of super that contains only our data.

Sparse format lets us use DONT_CARE chunks for the gaps, so the file holds just
the metadata (64 KiB x2) plus the six HarmonyOS images.

  make_sparse.py --out m0b/super_hmos.img
"""
import argparse
import os
import struct

BLK = 4096
SUPER_SIZE = 11274289152
SPARSE_MAGIC = 0xED26FF3A
CHUNK_RAW = 0xCAC1
CHUNK_DONT_CARE = 0xCAC3

HERE = "/mnt/e/1+10pro/1+10pro[HMOS]/m0b"
META = os.path.join(HERE, "lp_hmod_slot.bin")
OTA = os.path.join(HERE, "ota")

# (file, byte offset) - must match tools/build_lp.py
IMAGES = [
    ("system.img", 1048576),
    ("sys_prod.img", 3096444928),
    ("vendor_negroni.img", 4950327296),
    ("chip_prod.img", 5624561664),
    ("cust.img", 6204424192),
    ("patch.img", 6210715648),
]

META_OFF = 0x3000
META_SLOT1 = 0x13000


def blocks(n):
    return (n + BLK - 1) // BLK


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.join(HERE, "super_hmos.img"))
    a = ap.parse_args()

    meta = open(META, "rb").read()
    assert len(meta) == 0x10000, len(meta)

    # build the list of (offset, data) regions we must write
    regions = [(META_OFF, meta), (META_SLOT1, meta)]
    for name, off in IMAGES:
        p = os.path.join(OTA, name)
        regions.append((off, None))  # streamed below
    regions.sort(key=lambda r: r[0])

    total_blocks = SUPER_SIZE // BLK
    total_chunks = 0
    plan = []
    cursor = 0
    for off, data in regions:
        blk = off // BLK
        if blk > cursor:
            plan.append(("skip", blk - cursor))
            cursor = blk
        if data is None:
            name = None
            for n, o in IMAGES:
                if o == off:
                    name = n
            size = os.path.getsize(os.path.join(OTA, name))
            nb = blocks(size)
            plan.append(("raw", (os.path.join(OTA, name), nb, size)))
            cursor += nb
        else:
            nb = blocks(len(data))
            plan.append(("rawb", (data, nb)))
            cursor += nb
    if cursor < total_blocks:
        plan.append(("skip", total_blocks - cursor))
    total_chunks = len(plan)

    hdr = struct.pack("<IHHHHIIII", SPARSE_MAGIC, 1, 0, 28, 12, BLK,
                      total_blocks, total_chunks, 0)
    out = open(a.out, "wb")
    out.write(hdr)
    written = 0
    for kind, payload in plan:
        if kind == "skip":
            nb = payload
            ch = struct.pack("<HHII", CHUNK_DONT_CARE, 0, nb, 12)
            out.write(ch)
            written += 12
        elif kind == "rawb":
            data, nb = payload
            pad = nb * BLK - len(data)
            ch = struct.pack("<HHII", CHUNK_RAW, 0, nb, 12 + nb * BLK)
            out.write(ch)
            out.write(data)
            if pad:
                out.write(b"\0" * pad)
            written += 12 + nb * BLK
        else:
            path, nb, size = payload
            ch = struct.pack("<HHII", CHUNK_RAW, 0, nb, 12 + nb * BLK)
            out.write(ch)
            with open(path, "rb") as f:
                left = size
                while left:
                    buf = f.read(min(1 << 20, left))
                    if not buf:
                        break
                    out.write(buf)
                    left -= len(buf)
            pad = nb * BLK - size
            if pad:
                out.write(b"\0" * pad)
            written += 12 + nb * BLK
    out.close()
    print("wrote %s: %.2f GiB, %d chunks, %d blocks total" % (
        a.out, written / 2**30, total_chunks, total_blocks))
    print("file size on disk: %.2f GiB" % (os.path.getsize(a.out) / 2**30))


if __name__ == "__main__":
    main()
