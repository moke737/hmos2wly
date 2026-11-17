#!/usr/bin/env python3
"""Generate a valid Android A/B `misc` partition (bootloader_control block).

The struct is *not* what a naive reading suggests - it starts with the slot
suffix, not with the magic (that was the bug in the earlier hand-built image):

    offset  size  field
    0       4     slot_suffix[4]        "_a\\0\\0"
    4       4     magic (u32 LE)        0x42414c54  -> bytes 54 4c 41 42 "TLAB"
    8       1     version               1
    9       1     nb_slot               2
    10      1     recovery_tries_remaining
    11      1     merge_status
    12      2     reserved
    14      8     slot_info[4]  (2 bytes each: priority:4 tries:3 success:1
                                  verity:1 reserved:7)
    22      8     reserved2
    30      4     crc32_le              crc32 over bytes 0..29
    34      ...   reserved3 (zero, up to 4096)
    4096    ...   rest of the partition (zero)

Usage:
  gen_misc.py --out misc_good.img [--active a] [--size 1M]
"""
import argparse
import binascii
import struct

MAGIC = 0x42414C54
BOOT_CTRL_SIZE = 4096


def slot_meta(priority, tries, successful, verity_corrupted=0):
    # priority:4 | tries:3 | successful:1 | verity_corrupted:1 | reserved:7
    b0 = (priority & 0xF) | ((tries & 0x7) << 4) | ((successful & 0x1) << 7)
    b1 = (verity_corrupted & 0x1) | (0 << 1)
    return bytes([b0, b1])


def build(active="a", nb_slot=2):
    buf = bytearray(BOOT_CTRL_SIZE)
    suffix = ("_" + active).encode()[:3]
    buf[0:len(suffix)] = suffix
    struct.pack_into("<I", buf, 4, MAGIC)
    buf[8] = 1                      # version
    buf[9] = nb_slot
    buf[10] = 0                     # recovery_tries_remaining
    buf[11] = 0                     # merge_status
    # slot a: highest priority, 7 tries, marked successful
    buf[14:16] = slot_meta(15, 7, 1)
    # slot b: lower priority, 7 tries, not successful
    buf[16:18] = slot_meta(14, 7, 0)
    # AOSP uses the kernel crc32_le() which has no final inversion, while
    # binascii.crc32() is the standard CRC-32 (with inversion) -> undo it
    crc = (binascii.crc32(bytes(buf[:30])) ^ 0xFFFFFFFF) & 0xFFFFFFFF
    struct.pack_into("<I", buf, 30, crc)
    return bytes(buf), crc


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="misc_good.img")
    ap.add_argument("--active", default="a", choices=["a", "b"])
    ap.add_argument("--size", default="1M")
    a = ap.parse_args()

    size = a.size
    if size.endswith(("M", "m")):
        total = int(size[:-1]) << 20
    elif size.endswith(("K", "k")):
        total = int(size[:-1]) << 10
    else:
        total = int(size)

    blk, crc = build(a.active)
    img = blk + b"\0" * (total - len(blk))
    open(a.out, "wb").write(img)
    print("wrote %s (%d bytes)" % (a.out, len(img)))
    print("  slot_suffix=%r magic=0x%08x version=%d nb_slot=%d crc32=0x%08x"
          % (("_" + a.active), MAGIC, 1, 2, crc))
    print("  first 40 bytes: %s" % blk[:40].hex())
    # self-check
    chk = (binascii.crc32(blk[:30]) ^ 0xFFFFFFFF) & 0xFFFFFFFF
    stored = struct.unpack_from("<I", blk, 30)[0]
    print("  self-check: crc %s" % ("OK" if chk == stored else "MISMATCH"))


if __name__ == "__main__":
    main()
