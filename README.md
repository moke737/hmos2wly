# hmos2wly — HarmonyOS 5 on the OnePlus 10 Pro (SM8450)

**English** | [简体中文](README.zh-CN.md)

> Status: **work in progress**. The storage layer and the userspace bootstrap are
> done and verified on real hardware; the kernel side is the remaining blocker.
> Nothing here will boot into HarmonyOS yet — but everything below is verified.

---

## 1. What this is

A from-scratch attempt to bring the **HarmonyOS 5 (OpenHarmony userspace)**
boot architecture — the one pioneered by
[hmos2sweet](https://github.com/tsryzenstech/hmos2sweet) for Huawei devices — to
the **OnePlus 10 Pro** (`wly` / `negroni`, Qualcomm SM8450 / waipio).

The upstream project replaces a Huawei boot chain with:

```
Android bootloader
  └─ kernel (Android/LineageOS lineage, patched for OpenHarmony)
       └─ /init = hmos-preinit        <- maps super's logical partitions via
          │                               dm-linear, writes /etc/fstab.required,
          │                               execs the OpenHarmony first stage
          └─ /bin/init_early (OHOS)   <- HarmonyOS init, mounts /usr from super
```

This repository adapts that idea to a **Qualcomm** device, and documents the
hardware facts that a port of this kind depends on.

## 2. Current status

| Layer | State | Evidence |
|---|---|---|
| HarmonyOS partitions inside `super` | ✅ done | `dm-17` mounts and shows HarmonyOS files (`cfg_ady_al_va.ini`, …) |
| `hmos-preinit` userspace chain | ✅ done | dmesg: 182 device nodes → 5 dm-linear mappings → correct `fstab.required` → `exec /bin/init_early` |
| LP metadata generator (no `lpmake` needed) | ✅ done | OEM metadata parsed, SHA-256 scheme reproduced, kernel accepts the result |
| `vendor_boot` repacker | ✅ done | byte-identical self-test (`identical=True`), AVB footer preserved |
| Kernel: boot + UFS + OpenHarmony patches | ❌ blocked | see §5 |

### Milestone ladder

| Milestone | Meaning | State |
|---|---|---|
| A | kernel boots, OHOS init runs | kernel boots, but **no UFS yet** |
| B | display lights up | not started |
| C | touch / desktop | not started |
| D | daily usable (Wi-Fi, audio, modem) | not started |

## 3. Repository layout

```
tools/
  bootimg4.py            boot image (v3/v4) unpack/pack
  repack_vendor_boot.py  vendor_boot v4 repacker (OEM layout + AVB)
  build_lp.py            liblp metadata generator (template + SHA-256)
  gen_misc.py            Android A/B `misc` image generator
  make_sparse.py         build a sparse super image for fastboot
preinit/
  preinit.c              /init for HarmonyOS: mini-ueventd, module loader,
                         super LP → dm-linear, fstab.required, exec init_early
diag/
  init_v8_ramoops.c      diagnostic /init: dumps pstore + storage report
  init_v12_staged.c      diagnostic /init: failure stage encoded in the
                         power-off delay (for kernels with no log channel)
scripts/
  install_m0b.bat        write HarmonyOS partitions into super (TWRP + fastboot)
docs/
  donate-wechat.png      WeChat reward QR code
```

## 4. Quick start

### 4.1 Put HarmonyOS images into `super`

```bash
# 1. read the OEM metadata layout (dump the first 6 MiB of super in TWRP)
adb shell "dd if=/dev/block/by-name/super of=/tmp/super_head2.bin bs=1M count=6"
adb pull /tmp/super_head2.bin

# 2. generate metadata for the HarmonyOS partition set
python3 tools/build_lp.py --dump super_head2.bin --out lp_hmod_slot.bin

# 3. build a sparse super image (only our chunks take space)
python3 tools/make_sparse.py --out super_hmos.img

# 4. flash it (the bootloader writes it; TWRP's `dd` cannot - see §5.6)
fastboot flash super super_hmos.img
```

### 4.2 Run `preinit` without flashing a kernel

```bash
# in TWRP, with super already containing HarmonyOS
adb push preinit /tmp/preinit && adb shell "chmod 755 /tmp/preinit && /tmp/preinit"
adb shell "dmesg | grep hmos-preinit"      # mapping log
adb shell "cat /etc/fstab.required"        # generated mount table
```

### 4.3 Diagnose a kernel with no log channel

`diag/init_v12_staged.c` encodes its failure stage in the power-off delay:

```
delay = 10 + 5 * stage + module_load_failures   (capped at 120 s)
stage 0 = USB gadget up (stays alive)   stage 4 = no UDC
stage 1 = configfs mount failed         stage 5 = ttyGS0 missing
stage 2 = gadget dir failed
stage 3 = ACM function failed
```

So "the phone went dark after ~15 s" already tells you that every module loaded.

## 5. Technical findings (the useful part)

Everything below was measured on a NE2210 running ColorOS 17 / 16.0.3.520.

### 5.1 `super` LP metadata is vendor-extended and uses SHA-256, not CRC32

* geometry at `0x1000`, **52 bytes** (AOSP's is 24); a second copy at `0x2000`
* metadata slots at `0x3000 + n * 0x10000` (64 KiB each)
* tables start at `0x3100` (`header_size = 256`)
* partition entry = **52 bytes**: `name[36] attrs first_extent num_extents group`
* table counts live in the header, not inline
* **`0x30` holds `sha256(tables)`** (verified byte-exact over the 4812-byte table)
* `0x2C` holds `tables_size`; the 28 bytes at `0x10` are a header digest whose
  algorithm we could not identify — so the generator keeps the tables at
  **exactly the same length** (17 partitions / 155 extents / 3 groups / 1 block
  device) and only rewrites their contents plus the SHA-256.

That trick bypasses the unknown header digest entirely.

### 5.2 `vendor_boot` v4 layout (OnePlus flavour)

```
header(2128) | vendor ramdisk | dtb | bootconfig | pad | table |
pad | second bootconfig copy | pad(0x2000) | AVB vbmeta | … | AVB footer(64B)
```

* v4 header addresses are **u32**; `vendor_ramdisk_size` is at offset **24**
* the OEM image keeps a **second bootconfig copy** between table and vbmeta
* dropping the **AVB footer / vbmeta blob** makes ABL refuse the image and drop
  straight to fastboot
* `repack_vendor_boot.py --self-test` rebuilds the OEM image byte-identically

### 5.3 The kernel command line comes from `vendor_boot`, not `boot.img`

The stock `boot.img` cmdline is empty; `vendor_boot`'s cmdline is what the
kernel receives, and it contains

```
msm_geni_serial.con_enabled=0
```

i.e. **the OEM explicitly disables the GENI serial console**. Flip it to `1` (and
add `console=ttyMSM0,115200n8`) if you want a console.

### 5.4 `misc` is all zeros on this device

Dumped after a full line-flash: `misc` is 1024 KiB of zeros; OnePlus's ABL does
not keep A/B slot metadata there. Hand-writing `bootloader_control` is therefore
pointless — and `fastboot erase misc` is harmless. `tools/gen_misc.py` is kept
only as documentation of the AOSP structure (which starts with `slot_suffix`,
not with the magic, and needs the *kernel* CRC32 flavour: no final inversion).

### 5.5 `recovery` uses the kernel from `boot_a`

If `boot_a` holds a custom kernel that does not boot, TWRP cannot start either.
Always flash the stock `boot.img` back before booting recovery.

### 5.6 TWRP cannot write `super`

`dd` to `/dev/block/by-name/super` fails with `EPERM` even with SELinux
permissive and even on the raw node. Use `fastboot flash super <sparse image>`
instead — `tools/make_sparse.py` builds one with `DONT_CARE` chunks, so the file
only carries the data you actually need.

### 5.7 Kernel bisection results (OnePlus OSS tree, 5.10.236)

| Configuration | Result |
|---|---|
| phone's GKI config, no storage stack | ✅ boots, `/init` runs as PID 1 |
| + SMMU/IOMMU/ICC + UFS built in | ❌ panic loop |
| + SMMU/IOMMU with `iommu.passthrough=1` | ✅ boots |
| + `SERIAL_MSM_GENI` (GENI serial console) | ❌ hangs at the boot logo |
| + storage stack as 23 modules | ✅ boots |
| + `rpmhpd` (RPMh power domains) | ❌ hangs at the boot logo |

Conclusions:

* enabling **SMMU address translation** panics; `iommu.passthrough=1` avoids it
* the **GENI serial driver hangs this kernel** — which is why no console output
  was ever available
* `rpmhpd` hangs as a module; UFS needs the GCC clock controller
  (`gcc_ufs_phy_gdsc`), and GCC needs `rpmhpd` — that is the current deadlock
* `CONFIG_MODULES=n` breaks any driver that touches `struct module`
  (`debug_kinfo`, `minidump_log`, `soc_sleep_stats`, …)

### 5.8 Logging channels that do **not** work here

| Channel | Why not |
|---|---|
| UART `ttyMSM0` | no physical UART; driver hangs the kernel (§5.7) |
| `pstore` / `ramoops` | the OEM dtb uses `alloc-ranges` (address chosen per boot), so records never survive a reboot; the region can be pinned with an in-place dtb patch, but TWRP re-initialises it |
| raw `logdump` partition | needs UFS, which is exactly what is broken |
| framebuffer | no display driver in a minimal kernel |

## 6. Roadmap

1. get `rpmhpd` + `gcc-waipio` probing (try building them in rather than as
   modules, so they initialise in the normal `rpmh`/`cmd-db`/`scm` order)
2. verify UFS: `/dev/block/by-name/super` exists → `preinit` maps it
3. port the OpenHarmony kernel patch set (access_token, binder extensions,
   fs-verity stub, hmdfs emulation, memcg, DRM master auth, SELinux off)
4. display → touch → peripherals → network → audio → modem
   (each one is a separate device patch; expect the Huawei-specific HALs to be
   the hardest part)

## 7. Support

If this saved you some time, you can buy the author a coffee:

<img src="docs/donate-wechat.png" alt="WeChat reward QR" width="280">

## 8. Full engineering log

The complete measurement record (boot-chain analysis, the 30-patch device matrix,
the M0-M7 plan, every kernel bisection, and the dead ends) is in
[docs/FINDINGS.md](docs/FINDINGS.md) - written in Chinese, as it was kept day by day.

## 9. Credits & license

* Architecture and the preinit idea: [hmos2sweet](https://github.com/tsryzenstech/hmos2sweet)
* HarmonyOS 5 images: Huawei `update_full_base.zip` (Pura 70, `ADY-AL10`)
* Kernel sources: OnePlus OSS `android_kernel_oneplus_sm8450`,
  LineageOS `android_kernel_oneplus_sm8450` / `-modules` / `-devicetrees`

Scripts in this repository are **MIT** licensed (see `LICENSE`). Anything derived
from kernel sources stays under **GPL-2.0**. Huawei's proprietary images are
**not** distributed here.
