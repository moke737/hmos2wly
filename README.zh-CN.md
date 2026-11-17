# hmos2wly — 把 HarmonyOS 5 移植到一加 10 Pro（SM8450）

[English](README.md) | **简体中文**

> 状态：**进行中**。存储层和用户态引导已经在真机上完成并验证；**内核是当前的唯一拦路虎**。
> 现在还开不了机 —— 但下面每一条都是实测结论。

---

## 1. 这是什么

把 **HarmonyOS 5（OpenHarmony 用户态）** 的启动架构 —— 也就是
[hmos2sweet](https://github.com/tsryzenstech/hmos2sweet) 为华为设备做的那套 ——
搬到 **一加 10 Pro**（`wly` / `negroni`，高通 SM8450 / waipio）上。

上游项目把华为的启动链换成了：

```
Android bootloader
  └─ 内核（Android/LineageOS 血脉，打 OpenHarmony 补丁）
       └─ /init = hmos-preinit        ← 用 dm-linear 映射 super 里的逻辑分区，
          │                               生成 /etc/fstab.required，
          │                               然后 exec OpenHarmony 的一阶段 init
          └─ /bin/init_early (OHOS)   ← 鸿蒙 init，从 super 挂载 /usr
```

本仓库把同一思路搬到**高通平台**，并把这类移植所依赖的硬件事实全部记录下来。

## 2. 当前进度

| 层 | 状态 | 证据 |
|---|---|---|
| `super` 里的鸿蒙分区 | ✅ 完成 | `dm-17` 挂载后能看到鸿蒙文件（`cfg_ady_al_va.ini` 等） |
| `hmos-preinit` 用户态链路 | ✅ 完成 | dmesg：182 个设备节点 → 5 个 dm-linear 映射 → `fstab.required` 正确 → `exec /bin/init_early` |
| LP 元数据生成器（不需要 `lpmake`） | ✅ 完成 | 解析原厂元数据、复现 SHA-256 方案、内核接受生成结果 |
| `vendor_boot` 重打包工具 | ✅ 完成 | 自检 `identical=True`（逐字节还原），AVB footer 保留 |
| 内核：启动 + UFS + OpenHarmony 补丁 | ❌ 卡住 | 见 §5 |

### 里程碑阶梯

| 里程碑 | 含义 | 状态 |
|---|---|---|
| A | 内核启动、鸿蒙 init 跑起来 | 内核能启动，但**还没有 UFS** |
| B | 屏幕点亮 | 未开始 |
| C | 触控 / 桌面 | 未开始 |
| D | 日常可用（WiFi/音频/基带） | 未开始 |

## 3. 仓库结构

```
tools/
  bootimg4.py            boot 镜像（v3/v4）解包/打包
  repack_vendor_boot.py  vendor_boot v4 重打包（原厂布局 + AVB）
  build_lp.py            liblp 元数据生成器（模板法 + SHA-256）
  gen_misc.py            Android A/B `misc` 镜像生成器
  make_sparse.py         生成 sparse 格式的 super 镜像，供 fastboot 刷写
preinit/
  preinit.c              鸿蒙的 /init：迷你 ueventd、模块加载、super LP → dm-linear、
                         fstab.required、exec init_early
diag/
  init_v8_ramoops.c      诊断 init：把 pstore + 存储报告写进裸分区
  init_v12_staged.c      诊断 init：**用关机延时编码失败阶段**（内核没有日志通道时用）
scripts/
  install_m0b.bat        把鸿蒙分区写进 super（TWRP + fastboot）
docs/
  donate-wechat.png      微信赞赏码
```

## 4. 快速上手

### 4.1 把鸿蒙镜像写进 `super`

```bash
# 1. 在 TWRP 里 dump super 的前 6 MiB（原厂 LP 元数据模板）
adb shell "dd if=/dev/block/by-name/super of=/tmp/super_head2.bin bs=1M count=6"
adb pull /tmp/super_head2.bin

# 2. 生成鸿蒙分区集合的元数据
python3 tools/build_lp.py --dump super_head2.bin --out lp_hmod_slot.bin

# 3. 生成 sparse super 镜像（空白区域不占空间）
python3 tools/make_sparse.py --out super_hmos.img

# 4. 刷写（必须走 bootloader；TWRP 里的 dd 写不了 super，见 §5.6）
fastboot flash super super_hmos.img
```

### 4.2 不刷内核也能验证 `preinit`

```bash
# TWRP 里，且 super 已含鸿蒙分区
adb push preinit /tmp/preinit && adb shell "chmod 755 /tmp/preinit && /tmp/preinit"
adb shell "dmesg | grep hmos-preinit"      # 映射日志
adb shell "cat /etc/fstab.required"        # 生成的挂载表
```

### 4.3 给"没有日志通道"的内核做诊断

`diag/init_v12_staged.c` 把失败阶段编码进关机延时：

```
延时 = 10 + 5 × 阶段 + 模块加载失败数    （上限 120 秒）
阶段 0 = USB gadget 就绪（不关机）      阶段 4 = 找不到 UDC
阶段 1 = configfs 挂载失败              阶段 5 = ttyGS0 不存在
阶段 2 = gadget 目录创建失败
阶段 3 = ACM 功能创建失败
```

所以"手机大约 15 秒后变黑"本身就说明所有模块都加载成功了。

## 5. 实测技术结论（本仓库最有价值的部分）

全部在 NE2210（ColorOS 17 / 16.0.3.520）上测得。

### 5.1 `super` 的 LP 元数据是厂商扩展版，且用 SHA-256 而不是 CRC32

* geometry 在 `0x1000`，**52 字节**（AOSP 是 24），`0x2000` 还有一份
* 元数据槽在 `0x3000 + n × 0x10000`（每槽 64 KiB）
* 表格从 `0x3100` 开始（`header_size = 256`）
* 分区项 **52 字节**：`name[36] attrs first_extent num_extents group`
* 四个表的**计数在 header 里**，不在表内
* **`0x30` 处是 `sha256(表格区)`**（以 4812 字节跨度逐字节验证通过）
* `0x2C` 是 `tables_size`；`0x10` 处 28 字节是头部摘要，算法未识别 —— 所以生成器
  **保持表格长度完全不变**（17 分区 / 155 extent / 3 组 / 1 块设备），只改内容并重算 SHA-256

这个技巧让我们完全绕开了那个未知的头部摘要。

### 5.2 `vendor_boot` v4 的真实布局（一加风格）

```
header(2128) | vendor ramdisk | dtb | bootconfig | pad | table |
pad | 第二份 bootconfig | pad(0x2000) | AVB vbmeta | … | AVB footer(最后 64B)
```

* v4 头里的地址是 **u32**；`vendor_ramdisk_size` 在偏移 **24**
* 原厂镜像在 table 和 vbmeta 之间还有**第二份 bootconfig**
* 丢掉 **AVB footer / vbmeta 块**，ABL 会直接拒绝并掉进 fastboot
* `repack_vendor_boot.py --self-test` 能逐字节还原原厂镜像

### 5.3 内核 cmdline 来自 `vendor_boot`，不是 `boot.img`

原厂 `boot.img` 的 cmdline 是空的；内核实际收到的是 `vendor_boot` 的 cmdline，里面有

```
msm_geni_serial.con_enabled=0
```

也就是说**原厂明确关闭了 GENI 串口控制台**。想开就把 `0` 改成 `1`，并加
`console=ttyMSM0,115200n8`。

### 5.4 这台机器的 `misc` 全是 0

整包线刷后 dump：`misc` 是 1024 KiB 全零；一加 ABL 并不在这里存 A/B 槽位元数据。
所以手写 `bootloader_control` 毫无意义，`fastboot erase misc` 也无害。
`tools/gen_misc.py` 仅作为 AOSP 结构的记录保留（注意它**以 `slot_suffix` 开头**而不是
magic，且要用**内核版 CRC32**：不做最终取反）。

### 5.5 `recovery` 用的是 `boot_a` 里的内核

如果 `boot_a` 放的是无法启动的自编内核，TWRP 也起不来。
进 TWRP 之前务必先把官方 `boot.img` 刷回去。

### 5.6 TWRP 写不了 `super`

对 `/dev/block/by-name/super` 做 `dd` 会 `EPERM`，即使 SELinux permissive、即使直接写
裸节点也一样。改用 `fastboot flash super <sparse 镜像>`；`tools/make_sparse.py` 用
`DONT_CARE` 块生成稀疏镜像，文件里只包含真正要写的数据。

### 5.7 内核二分结果（一加 OSS 树，5.10.236）

| 配置 | 结果 |
|---|---|
| 手机原厂 GKI 配置，无存储栈 | ✅ 启动，`/init` 以 PID 1 运行 |
| + SMMU/IOMMU/ICC + UFS 内建 | ❌ panic 重启循环 |
| + SMMU/IOMMU，带 `iommu.passthrough=1` | ✅ 启动 |
| + `SERIAL_MSM_GENI`（GENI 串口控制台） | ❌ 卡在开机 logo |
| + 存储栈做成 23 个模块 | ✅ 启动 |
| + `rpmhpd`（RPMh 电源域） | ❌ 卡在开机 logo |

结论：

* 开启 **SMMU 地址翻译**会 panic，`iommu.passthrough=1` 可绕过
* **GENI 串口驱动会把内核挂死** —— 这就是一直拿不到 console 输出的原因
* `rpmhpd` 作为模块会挂；而 UFS 需要 GCC 时钟控制器（`gcc_ufs_phy_gdsc`），
  GCC 又需要 `rpmhpd` —— 这是当前的死结
* `CONFIG_MODULES=n` 会让所有引用 `struct module` 的驱动编译失败
  （`debug_kinfo`、`minidump_log`、`soc_sleep_stats` 等）

### 5.8 这台机器上**不可用**的日志通道

| 通道 | 原因 |
|---|---|
| UART `ttyMSM0` | 没有物理串口；驱动会把内核挂死（§5.7） |
| `pstore` / `ramoops` | 原厂 dtb 用 `alloc-ranges`（每次启动地址不同），记录无法跨重启；可以用原地 dtb 补丁固定地址，但 TWRP 启动时会重新初始化该区域 |
| 裸分区 `logdump` | 需要 UFS，而 UFS 正是坏的 |
| framebuffer | 最小内核里没有显示驱动 |

## 6. 路线图

1. 让 `rpmhpd` + `gcc-waipio` 能 probe（尝试改为内建，让它们在
   `rpmh`/`cmd-db`/`scm` 的正常初始化顺序里完成）
2. 验证 UFS：`/dev/block/by-name/super` 出现 → `preinit` 就能映射
3. 移植 OpenHarmony 内核补丁集（access_token、binder 扩展、fs-verity stub、
   hmdfs 模拟、memcg、DRM master 认证、关闭 SELinux）
4. 显示 → 触控 → 外设 → 网络 → 音频 → 基带
   （每一项都是独立的设备补丁；华为专属的 HAL 是最难的部分）

## 7. 支持作者

如果这个项目帮你省了时间，欢迎请作者喝杯咖啡：

<img src="docs/donate-wechat.png" alt="微信赞赏码" width="280">

## 8. 完整工程日志

逐日的完整实测记录（启动链分析、30 个设备补丁矩阵、M0–M7 计划、每一次内核二分、所有死路）在
[docs/FINDINGS.md](docs/FINDINGS.md)。

## 9. 致谢与许可

* 架构与 preinit 思路：[hmos2sweet](https://github.com/tsryzenstech/hmos2sweet)
* 鸿蒙 5 镜像：华为 `update_full_base.zip`（Pura 70，`ADY-AL10`）
* 内核源码：一加 OSS `android_kernel_oneplus_sm8450`，
  LineageOS `android_kernel_oneplus_sm8450` / `-modules` / `-devicetrees`

本仓库的脚本以 **MIT** 许可发布（见 `LICENSE`）。任何源自内核代码的部分保持
**GPL-2.0**。华为的专有镜像**不**在此分发。
