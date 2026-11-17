#!/usr/bin/env python3
"""Repack an Android v4 vendor_boot image (OnePlus/SM8450 flavour).

Layout discovered from the OEM image (it differs from the AOSP doc):

  header(2128) | vendor ramdisk | dtb | bootconfig | pad | table |
  pad | extra bootconfig text | pad(0x2000) | AVB vbmeta | ... | AVB footer

The AVB footer (last 64 bytes) and the vbmeta blob must be preserved, otherwise
OnePlus ABL refuses the image and drops straight to fastboot.

Usage:
  repack_vendor_boot.py --in vendor_boot.img --self-test
  repack_vendor_boot.py --in vendor_boot.img --out new.img --add-init /path/init
"""
import argparse
import lz4.block
import os
import struct
import sys


# ---------------------------------------------------------------- header ----
def parse_header(d):
    if d[0:8] != b"VNDRBOOT":
        raise SystemExit("not a vendor_boot image")
    # v4 vendor_boot uses u32 addresses (unlike boot_img_hdr v4)
    h = {}
    h["header_version"] = struct.unpack_from("<I", d, 8)[0]
    h["page_size"] = struct.unpack_from("<I", d, 12)[0]
    h["kernel_addr"] = struct.unpack_from("<I", d, 16)[0]
    h["ramdisk_addr"] = struct.unpack_from("<I", d, 20)[0]
    h["vendor_ramdisk_size"] = struct.unpack_from("<I", d, 24)[0]
    h["cmdline"] = d[28:28 + 2048].split(b"\0")[0].decode("utf-8", "replace")
    h["tags_addr"] = struct.unpack_from("<I", d, 2076)[0]
    h["name"] = d[2080:2096].split(b"\0")[0].decode("utf-8", "replace")
    h["header_size"] = struct.unpack_from("<I", d, 2096)[0]
    h["dtb_size"] = struct.unpack_from("<I", d, 2100)[0]
    h["dtb_addr"] = struct.unpack_from("<Q", d, 2104)[0]
    h["table_size"] = struct.unpack_from("<I", d, 2112)[0]
    h["table_num"] = struct.unpack_from("<I", d, 2116)[0]
    h["table_entry_size"] = struct.unpack_from("<I", d, 2120)[0]
    h["bootconfig_size"] = struct.unpack_from("<I", d, 2124)[0]
    return h


def align(x, page=4096):
    return (x + page - 1) // page * page


def layout(d, h):
    """(ramdisk_off, ramdisk_len, dtb_off, bootconfig_off, table_off)."""
    rd_off = align(h["header_size"], h["page_size"])
    rd_len = h["vendor_ramdisk_size"]
    dtb_off = align(rd_off + rd_len, h["page_size"])
    if d[dtb_off:dtb_off + 4] != b"\xd0\x0d\xfe\xed":
        pos = d.find(b"\xd0\x0d\xfe\xed", rd_off + rd_len)
        if pos < 0:
            raise SystemExit("dtb magic not found")
        dtb_off = pos
    bc_off = dtb_off + h["dtb_size"]
    table_off = align(bc_off + h["bootconfig_size"], h["page_size"])
    return rd_off, rd_len, dtb_off, bc_off, table_off


def avb_tail(d):
    footer = d[-64:]
    if footer[:4] != b"AVBf":
        return None, 0, None
    vb_off = struct.unpack_from(">Q", footer, 20)[0]
    vb_size = struct.unpack_from(">Q", footer, 28)[0]
    return vb_off, vb_size, footer


# ------------------------------------------------------------------ lz4 -----
def lz4_legacy_decompress(buf):
    out = bytearray()
    p = 0
    while p + 4 <= len(buf):
        magic = struct.unpack_from("<I", buf, p)[0]
        if magic != 0x184C2102:
            break
        p += 4
        while p + 4 <= len(buf):
            blk = struct.unpack_from("<I", buf, p)[0]
            p += 4
            if blk == 0:
                break
            chunk = buf[p:p + blk]
            p += blk
            try:
                out += lz4.block.decompress(chunk, uncompressed_size=1 << 26)
            except Exception:
                out += lz4.block.decompress(chunk)
    return bytes(out)


def lz4_legacy_compress(data, block=1 << 22):
    out = bytearray()
    out += struct.pack("<I", 0x184C2102)
    for i in range(0, len(data), block):
        piece = data[i:i + block]
        comp = lz4.block.compress(piece, mode="high_compression", store_size=False)
        out += struct.pack("<I", len(comp))
        out += comp
    out += struct.pack("<I", 0)
    return bytes(out)


# ------------------------------------------------------------------ cpio ----
def parse_cpio(b):
    entries = []
    p = 0
    while p + 110 <= len(b):
        if b[p:p + 6] not in (b"070701", b"070702"):
            break
        f = [int(b[p + 6 + i * 8:p + 6 + (i + 1) * 8], 16) for i in range(13)]
        namesize, filesize = f[11], f[6]
        name = b[p + 110:p + 110 + namesize - 1].decode("utf-8", "replace")
        p += 110 + namesize
        p = (p + 3) & ~3
        data = b[p:p + filesize]
        p += filesize
        p = (p + 3) & ~3
        if name == "TRAILER!!!":
            break
        entries.append({"name": name, "mode": f[1], "uid": f[2], "gid": f[3],
                        "nlink": f[4], "mtime": f[5], "data": data})
    return entries


def write_cpio(entries):
    out = bytearray()

    def hdr(name, mode, uid, gid, nlink, mtime, data):
        namez = name.encode() + b"\0"
        out.extend(b"070701")
        out.extend(b"%08X" % 0)          # ino
        out.extend(b"%08X" % mode)
        out.extend(b"%08X" % uid)
        out.extend(b"%08X" % gid)
        out.extend(b"%08X" % nlink)
        out.extend(b"%08X" % mtime)
        out.extend(b"%08X" % len(data))
        out.extend(b"%08X" % 0)          # devmajor
        out.extend(b"%08X" % 0)          # devminor
        out.extend(b"%08X" % 0)          # rdevmajor
        out.extend(b"%08X" % 0)          # rdevminor
        out.extend(b"%08X" % len(namez))
        out.extend(b"%08X" % 0)          # check
        out.extend(namez)
        while len(out) % 4:
            out.append(0)
        out.extend(data)
        while len(out) % 4:
            out.append(0)

    for e in entries:
        hdr(e["name"], e["mode"], e["uid"], e["gid"], e["nlink"], e["mtime"], e["data"])
    hdr("TRAILER!!!", 0, 0, 0, 1, 0, b"")
    return bytes(out)


# ----------------------------------------------------------------- build ----
def build(d, h, new_vendor_ramdisk):
    rd_off, rd_len, dtb_off, bc_off, table_off = layout(d, h)
    dtb = d[dtb_off:dtb_off + h["dtb_size"]]
    bootconfig = d[bc_off:bc_off + h["bootconfig_size"]]
    table = bytearray(d[table_off:table_off + h["table_size"]])
    if h["table_num"] >= 1:
        struct.pack_into("<I", table, 0, len(new_vendor_ramdisk))

    vb_off, vb_size, footer = avb_tail(d)
    vbmeta = d[vb_off:vb_off + vb_size] if footer else b""
    # the OEM keeps a second bootconfig copy between the table and the vbmeta
    extra = d[table_off + h["table_size"]:vb_off] if footer else b""

    hdr = bytearray(d[:h["header_size"]])
    struct.pack_into("<I", hdr, 24, len(new_vendor_ramdisk))   # vendor_ramdisk_size

    out = bytearray()
    out += hdr
    out += b"\0" * (align(len(out), h["page_size"]) - len(out))
    out += new_vendor_ramdisk
    out += b"\0" * (align(len(out), h["page_size"]) - len(out))
    out += dtb
    out += bootconfig
    out += b"\0" * (align(len(out), h["page_size"]) - len(out))
    out += table
    if footer:
        out += extra
        pad_to = (len(out) + 0x1fff) & ~0x1fff
        out += b"\0" * (pad_to - len(out))
        vb_off_new = len(out)
        out += vbmeta
        out += b"\0" * (len(d) - 64 - len(out))
        f = bytearray(footer)
        struct.pack_into(">Q", f, 12, vb_off_new)
        struct.pack_into(">Q", f, 20, vb_off_new)
        struct.pack_into(">Q", f, 28, vb_size)
        out += f
    elif len(out) < len(d):
        out += b"\0" * (len(d) - len(out))
    return bytes(out), (rd_off, dtb_off, bc_off, table_off)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="src", required=True)
    ap.add_argument("--out", dest="dst")
    ap.add_argument("--add-init")
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args()

    d = open(a.src, "rb").read()
    h = parse_header(d)
    rd_off, rd_len, dtb_off, bc_off, table_off = layout(d, h)
    print("vendor_boot: %d bytes, v%d page %d header %d" % (
        len(d), h["header_version"], h["page_size"], h["header_size"]))
    print("  vendor_ramdisk %d, dtb %d, table %d (%d x %d), bootconfig %d" % (
        rd_len, h["dtb_size"], h["table_size"], h["table_num"],
        h["table_entry_size"], h["bootconfig_size"]))
    print("  offsets: ramdisk 0x%x dtb 0x%x table 0x%x bootconfig 0x%x" % (
        rd_off, dtb_off, table_off, bc_off))
    vb_off, vb_size, footer = avb_tail(d)
    print("  avb: vbmeta 0x%x (%d bytes), footer %s" % (
        vb_off or 0, vb_size, "present" if footer else "MISSING"))

    raw = d[rd_off:rd_off + rd_len]
    print("  vendor ramdisk first bytes %s" % raw[:4].hex())
    cpio = lz4_legacy_decompress(raw)
    entries = parse_cpio(cpio)
    print("  cpio %d bytes, %d entries" % (len(cpio), len(entries)))

    if a.self_test:
        new_raw = lz4_legacy_compress(cpio)
        out, _ = build(d, h, raw)      # unchanged ramdisk -> must be identical
        same = out == d
        print("SELF-TEST: rebuilt %d bytes, identical=%s" % (len(out), same))
        if not same:
            for i in range(min(len(out), len(d))):
                if out[i] != d[i]:
                    print("  first difference at 0x%x" % i)
                    break
        return

    if a.add_init:
        init = open(a.add_init, "rb").read()
        entries = [e for e in entries if e["name"] != "init"]
        entries.append({"name": "init", "mode": 0o100755, "uid": 0, "gid": 0,
                        "nlink": 1, "mtime": 0, "data": init})
        new_cpio = write_cpio(entries)
        new_raw = lz4_legacy_compress(new_cpio)
        print("  + /init (%d bytes); cpio %d -> lz4 %d" % (
            len(init), len(new_cpio), len(new_raw)))
        out, _ = build(d, h, new_raw)
        if len(out) < len(d):
            out += b"\0" * (len(d) - len(out))
        open(a.dst, "wb").write(out)
        print("wrote %s (%d bytes)" % (a.dst, len(out)))
        h2 = parse_header(out)
        rd2, rl2, dtb2, bc2, tab2 = layout(out, h2)
        cpio2 = lz4_legacy_decompress(out[rd2:rd2 + h2["vendor_ramdisk_size"]])
        names2 = {e["name"] for e in parse_cpio(cpio2)}
        print("verify: init=%s modules.load=%s dtb=%s avb=%s" % (
            "init" in names2, "lib/modules/modules.load" in names2,
            out[dtb2:dtb2 + 4] == b"\xd0\x0d\xfe\xed",
            out[-64:-60] == b"AVBf"))


if __name__ == "__main__":
    main()
