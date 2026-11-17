#!/usr/bin/env python3
"""Generate HarmonyOS liblp metadata for super (equal-length tables trick).

The OEM header stores:
  * 0x2C : tables_size  (4812)
  * 0x30 : sha256(tables)   <- verified byte-exact against the OEM dump
  * 0x10 : 28-byte header digest (algorithm unknown)

We therefore keep the tables at *exactly* the same size (17 partitions,
155 extents, 3 groups, 1 block device) so that every header field - counts,
tables_size, header digest - stays valid, and only sha256(tables) is recomputed.

Layout produced:
  partitions[0..5] = system, sys_prod, vendor, chip_prod, cust, patch
                     (each 1 linear extent, group 1, attrs readonly)
  partitions[6..]  = num_extents 0 (kept as harmless placeholders)
  extents[0..5]    = our linear extents
  extents[6..]     = zero
"""
import argparse
import hashlib
import struct

HDR_OFF = 0x3000
TABLES_OFF = 0x3100
HDR_SIZE = 256
PART_SZ = 52
EXT_SZ = 24
GROUP_SZ = 48
BD_SZ = 64

PART_COUNT = 17
EXT_COUNT = 155
GROUP_COUNT = 3
BD_COUNT = 1
TABLES_SIZE = (PART_COUNT * PART_SZ + EXT_COUNT * EXT_SZ +
               GROUP_COUNT * GROUP_SZ + BD_COUNT * BD_SZ)   # 4812

SECTOR = 512
ALIGN_SECTORS = 2048          # 1 MiB, same as the OEM block device alignment
FIRST_LBA = 2048              # OEM super starts at LBA 2048

# name, size in bytes
PARTS = [
    ("system",    3094609920),
    ("sys_prod",  1853624320),
    ("vendor",     673529856),
    ("chip_prod",  579223552),
    ("cust",         6135808),
    ("patch",          20480),
]

TARGET_LINEAR = 0


def build(dump_path, out_path):
    d = open(dump_path, "rb").read()
    hdr = bytearray(d[HDR_OFF:HDR_OFF + HDR_SIZE])
    oem_tables = d[TABLES_OFF:TABLES_OFF + TABLES_SIZE]
    assert hashlib.sha256(oem_tables).digest() == bytes(hdr[0x30:0x50]), \
        "OEM tables digest mismatch - layout assumptions are wrong"
    print("OEM tables digest verified (sha256 @0x30)")

    # ---- parse OEM groups / block device so we keep them verbatim ----------
    gp_off = PART_COUNT * PART_SZ + EXT_COUNT * EXT_SZ
    groups = oem_tables[gp_off:gp_off + GROUP_COUNT * GROUP_SZ]
    bd_off = gp_off + GROUP_COUNT * GROUP_SZ
    bds = oem_tables[bd_off:bd_off + BD_COUNT * BD_SZ]
    print("group names:", [groups[i * GROUP_SZ:i * GROUP_SZ + 36].split(b"\0")[0].decode()
                           for i in range(GROUP_COUNT)])

    # ---- lay out our partitions -------------------------------------------
    lba = FIRST_LBA
    # in LP, first_lba is relative to the block device start; the OEM uses 2048
    extents = bytearray(EXT_COUNT * EXT_SZ)
    parts = bytearray(PART_COUNT * PART_SZ)
    total = 0
    for i, (name, size) in enumerate(PARTS):
        lba = (lba + ALIGN_SECTORS - 1) // ALIGN_SECTORS * ALIGN_SECTORS
        sectors = (size + SECTOR - 1) // SECTOR
        struct.pack_into("<QIII", extents, i * EXT_SZ, sectors, TARGET_LINEAR, lba, 0)
        nb = name.encode()
        parts[i * PART_SZ:i * PART_SZ + 36] = nb + b"\0" * (36 - len(nb))
        struct.pack_into("<IIII", parts, i * PART_SZ + 36, 1, i, 1, 1)  # attrs,first,num,group
        print("  %-10s %12d bytes -> %6d sectors at LBA %d (offset %d)" % (
            name, size, sectors, lba, lba * SECTOR))
        lba += sectors
        total += size
    print("total image bytes: %d (%.2f GiB), last LBA %d, super sectors %d" % (
        total, total / 2**30, lba, 11274289152 // SECTOR))
    assert lba * SECTOR < 11274289152, "does not fit in super"

    # placeholder partitions 6..16 keep num_extents = 0 (already zeroed)
    new_tables = bytes(parts) + bytes(extents) + groups + bds
    assert len(new_tables) == TABLES_SIZE, len(new_tables)

    # ---- header: only the tables digest changes ---------------------------
    struct.pack_into("<Q", hdr, 0x30, 0)          # not needed, we overwrite below
    hdr[0x30:0x50] = hashlib.sha256(new_tables).digest()
    # tables_size at 0x2C must stay 4812 (unchanged) - verify
    tsize = struct.unpack_from("<I", hdr, 0x2C)[0]
    assert tsize == TABLES_SIZE, "tables_size changed unexpectedly: %d" % tsize
    print("tables digest recomputed; tables_size field = %d" % tsize)

    slot = bytes(hdr) + new_tables
    slot = slot + b"\0" * (0x10000 - len(slot))
    open(out_path, "wb").write(slot)
    print("wrote %s (%d bytes = one metadata slot)" % (out_path, len(slot)))

    # ---- self check: re-parse what we just built --------------------------
    chk = open(out_path, "rb").read()
    chk_tables = chk[HDR_SIZE:HDR_SIZE + TABLES_SIZE]
    assert hashlib.sha256(chk_tables).digest() == chk[0x30:0x50]
    n = 0
    for i in range(PART_COUNT):
        e = i * PART_SZ
        name = chk[HDR_SIZE + e:HDR_SIZE + e + 36].split(b"\0")[0].decode()
        attrs, first, num, grp = struct.unpack_from("<IIII", chk, HDR_SIZE + e + 36)
        if num == 1:
            ex = HDR_SIZE + PART_COUNT * PART_SZ + first * EXT_SZ
            sec, tt, td, ts = struct.unpack_from("<QIII", chk, ex)
            print("  verify %-10s num=1 extent: %d sectors type=%d at LBA %d" % (
                name, sec, tt, td))
            n += 1
    print("self-check ok, %d real partitions" % n)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--dump", default="/mnt/e/1+10pro/1+10pro[HMOS]/m0b/super_head2.bin")
    ap.add_argument("--out", default="/mnt/e/1+10pro/1+10pro[HMOS]/m0b/lp_hmod_slot.bin")
    a = ap.parse_args()
    build(a.dump, a.out)
