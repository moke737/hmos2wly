# Engineering log — porting hmos2sweet to the OnePlus 10 Pro

> This is the raw working log kept during the port (in Chinese, as it was written
> day by day). It contains the full measurement record: boot-chain analysis, the
> 30-patch device matrix, the M0–M7 plan, every kernel bisection result, and the
> dead ends. `README.md` §5 is the condensed English version of the conclusions;
> this file is the evidence behind them.
>
> 这是移植期间逐日记录的完整工程日志（中文原文）。README 的 §5 是它的英文精华版，
> 本文件是结论背后的全部实测数据。

---
# hmos2sweet 鈫?涓€鍔?10 Pro锛圢E2210 / negroni锛夌Щ妞嶅彲琛屾€ц瘎浼?
- 鏃ユ湡锛?026-11-16
- 璁惧锛歂E2210 鍥借锛孋olorOS 17锛坄NE2210_17.0.0.107(CN01)`锛夛紝鍐呮牳 `5.10.236-android12-9` GKI锛屽凡瑙ｉ攣 / AVB 宸插叧 / TWRP + SukiSU
- 涓婃父锛?https://github.com/tsryzenstech/hmos2sweet>锛圚armonyOS 5 鈫?Redmi Note 10 Pro "sweet"锛?- 鏈湴浠撳簱锛歚E:\1+10pro\1+10pro[HMOS]\hmos2sweet2`锛?8.7 MB锛?1 涓ˉ涓佺洰褰曪紝鍚?docs/PATCHES.md 鍏ㄦ枃锛?
---

## 0. 缁撹

**鑳界Щ妞嶏紝浣嗕笉鏄?鏀硅澶囧悕"鐨勭Щ妞?鈥斺€?鏈川鏄?鍊熷畠鐨勬灦鏋勶紝閲嶅啓涓€涓?negroni 绉绘椤圭洰"銆?*

| | 鍒ゆ柇 |
|---|---|
| 鎶€鏈彲琛屾€?| 鉁?鍙锛堢敤鎴锋€佹槸鍗庝负鐜版垚鐨勶紝纭欢鍏ㄩ儴璧?Android 5.10 鍐呮牳閲屽凡鏈夌殑楠侀緳椹卞姩锛?|
| 涓?sweet 鐨勭浉浼煎害 | 浣庛€係oC 浠?SM7150/4.14 鍗曚綋鍐呮牳 鈫?SM8450/5.10 GKI + 妯″潡锛涙樉绀?1080p鈫?440p LTPO+DSC锛涢煶棰?tinyalsa鈫扐udioReach/AGM锛涚浉鏈?鈫扥ppo CamX |
| 鍙鐢ㄥ伐绋嬮噺 | 绾?30鈥?0%锛堣浆鎹㈠櫒銆乸reinit/super 鏄犲皠銆佽ˉ涓佹鏋躲€佹棤 TEE 鏂规銆佽嚜鐮?RIL/GNSS銆丮esa 鏂规銆佸埛鏈鸿剼鏈級 |
| "鑳借繘妗岄潰" | 鍗曚汉鐔熸墜 鈮?**2.5鈥? 浜烘湀**锛圡0鈥揗3 绾?1鈥?.5 涓湀鑳界湅鍒扮敾闈級 |
| "鑳芥棩鐢紙閫氳瘽/涓婄綉/鍚瓕锛? | 鈮?**6鈥? 浜烘湀**锛堥煶棰?AGM 鍜岀浉鏈烘槸涓昏宸环锛?|
| 灞忎笅鎸囩汗 | 鉂?涓嶅仛锛圱EE锛孡inux 鏃犻┍鍔紱sweet 涓婁篃娌″仛锛?|
| 鐜版垚鍏堜緥 | 鏈変汉涓€澶╃敤 AI 鏀瑰嚭 `pawlickerr/hmos2negroni`锛屼絾**娌℃湁浠讳綍寮€鏈鸿瘉鎹?*锛汼M8450 涓婃病鏈夋垚鍔熸渚?|

**涓€鍙ヨ瘽**锛氭瘮"鍐嶅仛涓€涓?sweet"闅句竴涓噺绾э紝浣嗘瘮浣犱粖骞村仛鐨?UniXNU/iOS 绉绘**椋庨櫓浣?*鈥斺€斿洜涓鸿繖娆′笉闇€瑕佽嚜宸卞啓纭欢椹卞姩锛岄┍鍔ㄩ兘鍦ㄥ巶鍟?5.10 鍐呮牳閲岋紝闅剧殑鏄妸 Android 涓棿浠讹紙AGM銆丆amX锛夋帴鍒伴缚钂欑殑 HDI 涓娿€?
---

## 1. 杩欎釜椤圭洰鍒板簳鍦ㄥ仛浠€涔堬紙涓夊眰锛?
```
绗?1 灞? 杞崲鍣紙Python锛宧mos2sweet.py + lib/锛?  鎷嗗崕涓?full OTA锛圥ura 70 / HarmonyOS 5.1 ADY-*锛寀pdate_full_base.zip锛?  system / vendor / sys_prod / chip_prod / cust / patch 鍘熸牱甯﹁蛋
  鍙敼 /vendor锛堜互鍙婂皯閲?system / sys_prod 鏂囦欢锛?
绗?2 灞? 鍐呮牳 + preinit
  LineageOS 4.14锛坰m6150锛? 18 涓?OpenHarmony 鍐呮牳琛ヤ竵
  boot.img锛歬ernel + HarmonyOS ramdisk锛?init = hmos-preinit
    鈫?璇?super 鐨?LP 鍏冩暟鎹紝缁欐瘡涓€昏緫鍒嗗尯寤?dm-linear
    鈫?鍐?/etc/fstab.required锛坋rofs锛?    鈫?鎸?cache锛岃捣 kmsg/hilog/screenshot 鏃ュ織
    鈫?androidboot.mode=charger 鏃惰繘鍏虫満鍏呯數寰幆
    鈫?exec HarmonyOS 鑷繁鐨?/bin/init_early

绗?3 灞? 纭欢鎶借薄锛?1 涓ˉ涓侊紝patches/NN-name锛?  鎶?Kirin 閭ｄ竴灞?缈昏瘧"鎴愰獊榫欙細
  display锛圖RM/KMS composer + dma-buf allocator VDI锛夈€乬pu锛圡esa freedreno on kgsl锛夈€?  audio锛坱inyalsa VDI锛夈€乺emoteproc锛坬rtr/rmtfs/tqftpserv/pd-mapper锛夈€?  wifi锛坕cnss+bdwlan锛夈€乥luetooth銆乻ensors锛坔exagonrpcd + SSC/QMI锛夈€?  RIL/GNSS锛堣嚜鐮?QMI 瀹㈡埛绔級銆乧amera锛坆ionic 閲岃窇 CamX锛屽啀妗ュ洖 OHOS camera HDI锛?```

**鏍稿績鎬濇兂**锛氶缚钂欑敤鎴锋€佷竴琛屼笉鏀癸紝鎶婂畠鑴氫笅閭ｅ眰"鍗庝负椹卞姩"鏁翠釜鎹㈡帀銆傛墍浠?绉绘" = 閲嶅啓绗?3 灞?+ 鎹㈢ 2 灞傜殑鍐呮牳/璁惧鏍戯紝绗?1 灞傚嚑涔庨€氱敤銆?
---

## 2. 杩欏彴鏈哄櫒鐨勬潯浠讹紙瀹炴祴鏁版嵁锛?
| 椤?| NE2210 瀹炴祴 | 瀵圭Щ妞嶇殑褰卞搷 |
|---|---|---|
| 瑙ｉ攣 / AVB | `unlocked=yes`锛沗avbctl` 鏄剧ず verity/verification 鍧?disabled锛泇bmeta 鐢?`--flags 3` 鐢熸垚 | 鐪佹帀鏈€闅剧殑涓€姝ワ紱iOS 绉绘宸查獙璇?ABL 鍙 **header v4** + vbmeta 蹇呴』涓?boot/vendor_boot 鍖归厤 |
| 妲戒綅 | A/B锛坴irtual A/B锛夛紝`current-slot=a`锛宻uper 涓ゆЫ鍏变韩 | **涓嶈兘鍙岀郴缁?*锛泂uper 涓€鏀瑰氨鏄叏娓?|
| 寮曞鐩稿叧鍒嗗尯 | `boot` / `vendor_boot` / `dtbo` / `recovery`锛?*鏃?init_boot**锛寁4 header | vendor_boot 閲屾槸 DTB + vendor ramdisk锛堟ā鍧楋級锛屽繀椤讳繚鐣?|
| super | 11,274,289,152 B锛?0.50 GiB锛夛紝group 涓婇檺 11,270,094,848 B | 瑙?搂6 |
| ColorOS 閫昏緫鍒嗗尯 | 9.92 GiB锛歮y_stock 4.62G / odm 1.29G / my_product 1.22G / system 1.03G / system_ext 0.94G / vendor 0.70G / product 0.15G 鈥︼紝浣欓噺 586.8 MiB | 鍒?my_stock+odm+my_product 鑵惧嚭 鈮?.1G锛屽楦胯挋鐢?|
| 鍐呮牳 | 5.10.236-android12-9锛圙KI android12-5.10锛夛紱鍘傚晢椹卞姩鏄?*妯″潡**锛屽垎甯冨湪 vendor_boot 鐨?vendor ramdisk 鍜?vendor_dlkm / odm_dlkm | 涓?sweet 鐨?4.14 鍗曚綋鍐呮牳瀹屽叏涓嶅悓锛堣 搂4.1锛?|
| 灞忓箷 | 3216脳1440 LTPO AMOLED锛孲amsung AMB670YF07 + S6E3HC3 DDIC锛屽崟 DSI 4 lane + **DSC**锛屼寒搴﹁蛋 panel DCS锛坄/sys/class/backlight`锛?| composer 闇€閫傞厤 DSC / 澶氬埛鏂扮巼 / 浜害 |
| 闊抽 | WCD9385 codec + 2脳 NXP TFA9874 鍔熸斁锛屽唴鏍告槸 **AudioReach**锛圙PR + AGM/GSL锛?| sweet 鐨?tinyalsa + mixer_paths 鏂规**涓嶆垚绔?*锛堣 搂4.2锛?|
| 鐩告満 | IMX789锛堜富锛? S5KJN1锛堣秴骞匡級+ OV08A10锛堥暱鐒︼級锛孫ppo 瀹氬埗 CamX | 鏈€璐电殑涓€鍧?|
| 鎸囩汗 | 灞忎笅鍏夊锛屾棤 Linux 椹卞姩 | 涓嶅仛 |
| 鍩哄甫 | X65 + IPA v5.1 | RIL 浠ｇ爜閫氱敤锛屽浐浠?IPA 瑕佹敼 |
| 宸叉湁鎴愭灉 | **UniXNU 鐪熸満杩涗簡 iOS 16 妗岄潰**锛氭樉绀?瑙︽帶/鎸夐敭鍙敤锛汫PU 鍗″湪 ZAP/PAS锛坄PAS supported: gpu 0`锛夛紝WiFi(QCA6490) 鍗″湪 BHI 鍚?`MHISTATUS` 涓?READY | 寮曞閾俱€佸埛鍐欍€佹棩蹇椼€佸洖婊氬璺凡鎵撻€氾紱GPU/WiFi 鐨勫潙宸叉帰鏄庯紝涓旂煡閬撳摢浜涙槸 mainline 鐗规湁銆佸摢浜涙槸鐪熷潙 |

---

## 3. 31 涓ˉ涓佺殑鍙Щ妞嶆€х煩闃?
### A. 鍑犱箮鐩存帴鍙敤锛堟敼鍙傛暟/閰嶇疆锛屸墹 2 澶?涓級

| 琛ヤ竵 | 璇存槑 |
|---|---|
| `12-display-geometry` | 鏀?1440脳3216 / ~522 dpi / 鎸栧瓟浣嶇疆 |
| `13-refresh-120hz` | 鏀?HGM 妯″紡琛ㄤ笌鍒锋柊鐜囩瓥鐣?|
| `14-boot-performance`銆乣16-performance` | 鍙傛暟绫?|
| `15-no-crash-reboot` | 閫氱敤锛屼笖**寮虹儓寤鸿榛樿寮€**锛堢Щ妞嶆湡鏈嶅姟蹇呭穿锛?|
| `20-userauth-no-tee`銆乣21-huks-software`銆乣46-pin-lock` | 鏃?TEE 鐨勯攣灞?PIN/HUKS 鏂规锛岄€氱敤锛岄噸缂栧嵆鍙?|
| `22-camera-daemon-idle` | 鍗犱綅 stub |
| `37-kirin-daemons-off` | 鍏虫帀 Kirin 涓撳睘瀹堟姢杩涚▼锛?0 Pro 涓婃洿璇ュ叧 |
| `41-chipset-vsync-stub` | 閫氱敤 stub |
| `50-remove-setup-wizard` | 棣栧惎娌℃湁 WiFi 鏃跺繀寮€ |
| `60-debug-tools` | 閫氱敤 |

### B. 瑕佹敼锛堣澶囩浉鍏筹紝涓瓑锛屾暟澶╁埌 1鈥? 鍛?涓級

| 琛ヤ竵 | 瑕佹敼浠€涔?|
|---|---|
| `00-core` | preinit 閫昏緫閫氱敤锛涙敼 cmdline锛坄hardware=negroni`銆乁FS/geni 鍩哄潃锛夈€乣fstab.negroni`銆乻uper 鍒嗗尯琛ㄣ€丟KI 妯″潡 insmod銆乧ache 鏃ュ織鍥為€€ |
| `10-display` | composer/allocator 鏄€氱敤 DRM atomic锛坙ibdrm锛夛紝鍙惉锛涜澶勭悊 DSC銆佸崟 DSI銆?0/90/120 妯″紡銆丼DE writeback 杩囨护銆佷寒搴﹁妭鐐?|
| `30-remoteproc` | qrtr/rmtfs/tqftpserv/pd-mapper 涓婃父閫氱敤锛沗sweet-rproc` 瑕佹敼鍥轰欢鍒嗗尯銆乵odem 鍚嶃€両PA v5.1銆丄DSP |
| `34-sensors-pd` | hexagonrpcd 閫氱敤锛屾崲 SSC 鐨?registry/config/ACDB |
| `39-sensors-hal` | VDI 閫氱敤锛孲UID / 棰戠巼琛ㄦ寜 10 Pro 鏀?|
| `36-thermal` | 閲嶅啓 thermal zone 鍒楄〃涓庣瓥鐣?|
| `38-usb` | UDC 鍚嶏紙`a600000.usb`锛夈€乣init.negroni.usb.cfg` |
| `42-ril` | 鑷爺 QMI 瀹㈡埛绔笌 SoC 鏃犲叧锛屾崲 modem 鍥轰欢/NV/APN/鍗￠厤缃?|
| `44-gnss` | QMI LOC锛岄€氱敤锛屾崲 SUID |
| `45-nfc` | 鎹?10 Pro 鐨?NFC 鑺墖閰嶇疆锛堜笉鏀寔灏辫烦杩囷級 |
| `33-haptics` | 鎹㈤┈杈鹃┍鍔?+ `/sys/class/leds/vibrator` 鏆撮湶 |
| `35-bluetooth` | 鎹?BT 鑺墖/浼犺緭灞傦紱10 Pro 鏃?3.5mm锛岄煶棰戣蛋 A2DP/USB |

### C. 瑕侀噸鍐欙紙鍥伴毦锛?
| 琛ヤ竵 | 闅剧偣 |
|---|---|
| `11-gpu-mesa` | Adreno 730 + **kgsl**锛堜笉鏄?msm DRM锛夈€侻esa 26 freedreno 鏀寔 a7xx锛圙LES 3.2/GL 4.5锛夛紝浣?a7xx-on-kgsl 鍦?OHOS 閲屾病浜洪獙璇佽繃锛涘厹搴?kms_swrast 杞欢娓叉煋 |
| `31-audio` | AudioReach/AGM/GSL 鏋舵瀯绾у樊寮傦紝瑙?搂4.2锛?鈥? 鍛?|
| `32-wifi` | cnss2/qcacld + BDF + MAC锛涘唴鏍镐晶鎶?BDF 涓嬭浇鎼繘鏉ワ紙sweet 鏄敼 icnss锛?0 Pro 鏄?cnss2锛?|
| `47-camera` | 122 涓?blob 鍏ㄦ崲 + 涓夋憚澶氭祦 + Oppo 瀹氬埗锛?鈥? 鏈?|

### D. 璺宠繃

| 琛ヤ竵 | 鍘熷洜 |
|---|---|
| `43-consumer-ir` | 10 Pro 鏃犵孩澶?|
| `40-hevc-4k-experimental` | 鍙€夊疄楠岄」 |

缁熻锛?*~10 鐩存帴 / ~13 閫傞厤 / 4 閲嶅啓 / 2 璺宠繃**銆?
---

## 4. 浜斾釜纭澶达紙鎸夐闄╂帓搴忥級

### 4.1 鍐呮牳锛?.14 鍗曚綋 鈫?5.10 GKI + 妯″潡

sweet 鏄?LineageOS 4.14 鍗曚綋鍐呮牳锛?0 Pro 鏄?GKI 5.10 + 妯″潡锛坴endor_boot 绗竴闃舵妯″潡 + vendor_dlkm/odm_dlkm 绗簩闃舵锛夈€?
涓ゆ潯璺細

- **(a) 鍗曚綋鍖?*锛氱敤 OnePlusOSS `sm8450` 鏍戯紙浣犵紪 SukiSU 鐢ㄧ殑閭ｅ锛夋墦 OHOS 琛ヤ竵锛屽敖閲忔妸鏄剧ず/kgsl/鍧楄澶?鏂囦欢绯荤粺璁炬垚 `=y`锛屽噺灏戞ā鍧椾緷璧栥€傚 OHOS 鏈€绠€鍗曪紝鎺ㄨ崘鍏堣蛋杩欐潯銆?- **(b) 淇濈暀 GKI + 妯″潡**锛氭ā鍧楃暀鍦?vendor_boot 鐨?vendor ramdisk锛宲reinit 閲?`insmod`銆傛洿鎺ヨ繎鍘熷巶锛屼絾渚濊禆椤哄簭瑕佽嚜宸辩銆?
18 涓?OHOS 琛ヤ竵瑕?rebase 鍒?5.10锛圤penHarmony 涓荤嚎鏈韩鏈?5.10 鐗堣ˉ涓侀泦锛屼富瑕佹槸閫傞厤 msm-5.10 鐨?binder/binderfs-off銆乤ccess_token銆乫s-verity stub銆乭mdfs 妯℃嫙銆乵emcg銆丏RM master銆乫irmware path銆乮cnss鈫抍nss2锛夈€傚伐浣滈噺闆嗕腑鍦?0001/0002/0003 涓変釜"鐢ㄦ埛鎬?bring-up"琛ヤ竵銆?
### 4.2 闊抽锛欰udioReach 鏄灦鏋勭骇宸紓锛堢涓€纭澶达級

- waipio/negroni 鐨勫唴鏍告槸 **AudioReach**锛歅CM 涓嶈兘鐩存帴 `pcm_open` 灏卞嚭澹帮紝蹇呴』鍏堢敱鐢ㄦ埛鎬侊紙AGM/GSL + 鎷撴墤锛夋妸 FE鈫払E 鐨勫浘閰嶅ソ锛泂weet 閭ｅ "tinyalsa + `mixer_paths_idp.xml` 璺敱" 鍦?5.10 涓婁笉鎴愮珛銆?- 鐜板疄璺嚎锛氭妸楂橀€氱殑 Android 闊抽 HAL/AGM 瀹㈡埛绔斁杩?鏈€灏?Android 鍛藉悕绌洪棿"锛孫HOS 鐨?audio VDI 鍙樻垚瀹冪殑浠ｇ悊 鈥斺€?灏辨槸浠栦滑缁欑浉鏈哄仛鐨?`cambridge` 鍚屾鎬濊矾銆傚０闊?閫氳瘽/褰曢煶閮借繃杩欐潯璺€?- 杩欐槸鍐冲畾"鑳戒笉鑳芥棩鐢?鐨勭涓€鍧楋紱涔熸槸 hmos2negroni 瀹屽叏娌＄鐨勯儴鍒嗭紙瀹冨彧鏄妸 sound 缂栬繘鍐呮牳锛夈€?
### 4.3 鏄剧ず锛?440p LTPO + DSC

- composer 浠ｇ爜鏄€氱敤 DRM atomic锛屽彲鎼紱闅剧殑鏄潰鏉匡細鍗?DSI + DSC銆?0/90/120 妯″紡銆丩TPO 鍙橀銆佷寒搴﹁蛋 panel DCS锛堜笉鏄櫘閫?backlight锛夛紝浠ュ強 sys_prod 閲岀殑 dpi/鎸栧瓟/HGM 绛栫暐瑕佹寜 3216脳1440 閲嶅啓銆?- 濂芥秷鎭細iOS 绉绘閲岃繖鍧楀睆鍦ㄩ潪 Android 鍐呮牳涓婂嚭杩囧浘锛汸MOS/mainline 渚у凡鏈?negroni 鐨?panel 椹卞姩锛坄panel_samsung_amb670yf07_fs_1440_3216_dsc_cmd`锛夛紝妯″紡/鏃跺簭鏈夋嵁鍙煡銆?
### 4.4 GPU锛欰dreno 730

- Mesa freedreno 鏀寔 a730锛圙LES 3.2 / GL 4.5锛夛紱**mainline msm** 璺緞鍦?negroni 涓婅窇杩?Plasma/GNOME 鐨?3D銆?- 浣嗘湰鏂规瑕佺敤 Android 5.10 鐨?**kgsl**锛堜笉鏄?msm DRM锛夛紝a7xx-on-kgsl 鏄?鑳界敤浣嗘病浜哄湪 OHOS 楠岃瘉杩?鐨勫尯鍩熴€傚浐浠朵笁浠跺锛坄a730_zap.mdt/.mbn`銆乣gmu_gen70000.bin`銆乣a730_sqe.fw`锛変綘宸蹭粠 `vendor.img` 鎶藉嚭銆?- 鍏滃簳锛氬厛鐢?Mesa `kms_swrast`锛堣蒋浠?GL锛夋妸 render_service 鎷夎捣鏉ワ紝UI 鑳借窇浣嗘參锛孏PU 鍚庣疆銆?- 浣犲湪 iOS 閭ｈ竟鐪嬪埌 `PAS supported: gpu 0` / SCM 鍗℃锛屾槸鑷啓鍐呮牳鑷繁璧?ZAP 鐨勭粨鏋滐紱鐢ㄥ巶鍟?5.10 鍐呮牳鐨?kgsl 璧扮殑鏄?ColorOS 鍚屼竴鏉¤璇佽矾寰勶紝鐞嗚涓婅兘杩?鈥斺€?**浣嗚繖鏄?M2 绗竴涓楠岃瘉鐨勭偣**銆?
### 4.5 鐩告満 + 鎸囩汗

- 鐩告満锛欼MX789 涓夋憚 + Oppo CamX銆俙cambridge` 鎬濊矾鍙鐢紙bionic 閲岃窇 camera3 HAL + dma-buf 浜ゆ帴 + OHOS camera HDI 瀹夸富锛夛紝浣嗙瓑浠蜂簬鎶?sweet 鐨?122 涓?blob 鍏ㄦ崲鎴?10 Pro 鐨勶紝骞堕噸鍋氫笁鎽?澶氭祦/JPEG/OIS銆傛渶鍚庡仛锛涗笉鍋氫篃鑳芥棩鐢紙缂烘壂鐮?鎷嶇収锛夈€?- 鎸囩汗锛氬睆涓嬪厜瀛?+ TEE锛孡inux 鏃犻┍鍔ㄣ€俿weet 涓婁篃娌″仛锛堥攣灞忕敤 PIN锛岄潬 20/21/46 涓変釜琛ヤ竵鐨勬棤 TEE 鏂规锛夈€?
---

## 5. 寮曞閾句笌鍒峰啓鏂规

鍙互鐩存帴鐓т綘 iOS 閭ｅ閰嶆柟鏀癸紙閭ｅ宸茬粡鐪熸満楠岃瘉杩?v4 header / vbmeta / slot 鎭㈠锛夛細

| 姝?| 鍐呭 | 澶囨敞 |
|---|---|---|
| 1 | `boot_a`锛歷4 header锛?.10 鍐呮牳 + HarmonyOS ramdisk锛宍/init = hmos-preinit` | 鍗曚綋鍐呮牳鏈€绠€鍗?|
| 2 | `vendor_boot_a`锛?*淇濈暀鍘熷巶 DTB/DTB overlay + vendor ramdisk锛堟ā鍧楋級** | 鍒収 iOS 閭ｆ牱鎼炵┖ vendor ramdisk锛堥偅鏄粰 mainline 鐨勶級 |
| 3 | `dtbo_a`锛?*淇濈暀鍘熷巶** | iOS 閭ｅ鏄摝鎺?dtbo 鐨勶紝Android 鍐呮牳鐩稿弽锛岃鐣欑潃 |
| 4 | `vbmeta_a` / `vbmeta_system_a` / `vbmeta_vendor_a`锛歚avbtool make_vbmeta_image --flags 3` | 浣犲凡楠岃瘉 flags=3 鑳借繃 ABL |
| 5 | `super`锛氫簩閫変竴 | (a) fastbootd `create-logical-partition`锛堜綘鍒犺繃 36 涓?cow锛宑reate 鏈獙璇侊級锛?b) 鐩存帴鐢熸垚鏁寸洏 `super.img` 鍐欒繘 `super` 鍒嗗尯锛坔mos2negroni 閫変簡杩欐潯锛岀粫寮€ fastbootd 闄愬埗锛?|
| 6 | `fastboot set_active a` 鈫?reboot | |
| 7 | 鏃ュ織锛歱reinit 鍐?cache锛堣嫢鏃?cache 鍒嗗尯灏卞啓鍒/pstore锛夛紝recovery 閲?adb pull | 娌跨敤 `get_logs` |
| 8 | 鍥為€€锛氭暣鍖?ColorOS OFP锛堜綘宸叉湁 17.0.0.107 鐨勫寘鍜岃剼鏈級 | 蹇呯暀鍚庤矾 |

**娉ㄦ剰**

- ABL 瀵?boot/vendor_boot/vbmeta 涓€鑷存€ф晱鎰燂細鏀逛竴澶勫氨瑕佸悓姝?vbmeta锛屽惁鍒?`slot-unbootable`锛堜綘 iOS 浜嬫晠璁板綍閲岃俯杩囷級銆?- **涓嶈纰?`qbootctl`**锛歭inux-msm 鐨?issue 鎶ュ憡 15.0.0.1302 涔嬪悗浼氬彉鐮栵紝EDL 瑕佷粯璐广€?- super 閲嶅啓 = ColorOS 鍏ㄩ儴閫昏緫鍒嗗尯 + COW 娓呯┖ = 娓呮満锛沗/data` 涔熻 format f2fs銆?- 楦胯挋渚ф病鏈?Android 鐨?bootloader control HAL锛宍slot-successful` 涓嶄細琚爣璁帮紱杩炵画澶辫触 ABL 鍙兘鏍?unbootable锛岀敤 `fastboot set_active a` 娓呫€?- 棣栨寮€鏈哄悜瀵艰 WiFi锛堝彲鐢ㄨˉ涓?50 鍘绘帀锛夛紝HarmonyOS 婵€娲?搴旂敤瀹夎瑕佸崕涓鸿处鍙枫€?
---

## 6. 瀹归噺涓庢暟鎹?
- super 10.50 GiB锛汣olorOS 閫昏緫鍒嗗尯 9.92 GiB锛屼綑閲?586.8 MiB銆?- 鍗庝负 full OTA 鍘嬬缉鍖?5.68 GB锛圥ura 70 `ADY-LGRP3-CHN 205.1.0.128(SP5)`锛宻ha256 瑙?`lib/fetchfw_catalog.json`锛夛紱瑙ｅ嚭鐨?system/vendor/sys_prod/chip_prod/cust/patch 浼?**7鈥? GB**锛堥渶瀹炴祴锛夈€?- 蹇呴』鍒狅細`my_stock` 4.62G + `odm` 1.29G + `my_product` 1.22G 鈮?7.1G锛屽锛涗笉澶熷啀鍒?`system_ext`/`product`/鍏朵綑 `my_*`銆?- `/data`锛歠ormat f2fs锛圓ndroid 鏁版嵁鍏ㄦ竻锛夈€?- 宸ヤ綔鐩橈細瑙ｅ寘 + 涓棿鏍?+ 杈撳嚭 鈮?**25 GB**锛圵SL2锛屽鐢ㄤ綘 iOS 閭ｅ WSL 娴佺▼锛夈€?
---

## 7. 闃舵璁″垝锛圡0鈥揗7锛?
| 闃舵 | 鐩爣 | 楠屾敹鏍囧噯 | 浼版椂 | 椋庨櫓 |
|---|---|---|---|---|
| **M0** | 寮曞 PoC锛氬唴鏍?+ preinit 鎶?HarmonyOS `init` 鎷夎捣鏉ワ紙鍙互榛戝睆锛?| kmsg/hilog 閲岃兘鐪嬪埌 `hmos-preinit ...` 鍜?`init_early` 鐨勮緭鍑?| 2鈥? 澶?| 涓紙鍐呮牳鑳藉惁缂栬繃銆乻uper 甯冨眬锛?|
| **M1** | 鏄剧ず + 瑙︽帶 | 浜睆銆?0Hz銆佹懜寰楀埌 | 1鈥? 鍛?| 涓?|
| **M2** | GPU | Mesa a730锛堝厛 swrast 鍏滃簳锛?| 1鈥? 鍛?| 楂?|
| **M3** | 鍩虹澶栬 | 鐢垫簮/鍏呯數/鎸夐敭/鎸姩/USB/鐑?姝ｅ父 | 1鈥? 鍛?| 浣?|
| **M4** | 缃戠粶 | WiFi + BT | 1鈥? 鍛?| 涓?|
| **M5** | 闊抽 | 澶栨斁/鍚瓛/鑰虫満/楹﹀厠椋?閫氳瘽璺敱 | 3鈥? 鍛?| 楂?|
| **M6** | 鍩哄甫 | SIM/娉ㄥ唽/鐭俊/鏁版嵁/GNSS/VoLTE锛堥€氳瘽渚濊禆 M5锛?| 2鈥? 鍛?| 涓珮 |
| **M7** | 鐩告満 | 棰勮 + JPEG锛堜笁鎽勶級 | 2鈥? 鏈?| 寰堥珮 |
| 鈥?| 鎸囩汗 | 涓嶅仛 | 鈥?| 鈥?|

M0鈥揗6 绱 鈮?**2.5鈥? 浜烘湀**锛汳7 鍙﹀姞 2鈥? 浜烘湀銆備笟浣欐椂闂村仛锛屾寜鍗婂勾鍒颁竴骞磋鍒掋€?
---

## 8. 椋庨櫓娓呭崟

1. **鍙樼爾/鍥炴粴**锛歴uper/boot 鏀瑰潖 鈫?OFP 鍏ㄩ噺鍥炲埛锛涗笉纰?`abl`/`xbl`/`qbootctl`锛汚RB index 鏈煡銆?2. **ABL 涓€鑷存€?/ slot-unbootable**锛氳€侀棶棰橈紝iOS 绉绘宸茶俯杩囷紝鏈夎В娉曘€?3. **HarmonyOS 鏄崕涓洪棴婧愮敤鎴锋€?*锛氫粎鑷敤锛涙縺娲昏 WiFi + 鍗庝负璐﹀彿锛涚郴缁?OTA 浼氬皾璇曞崌绾э紝瑕佸叧鎺夈€?4. **鏃?TEE**锛氭寚绾广€佹敮浠樼骇瀵嗛挜銆侀儴鍒?DRM 涓嶅彲鐢紙sweet 鍚屾牱锛夈€?5. **闊抽/鐩告満鏄?鑳藉紑鏈?鍜?鑳芥棩鐢?涔嬮棿鐨?3 涓湀**銆?6. **鐢熸€?*锛欻armonyOS 5 (NEXT) 鍙 `.hap`锛屾病鏈?Android 鍏煎灞傦紱鑳借鐨勫彧鏈夊崕涓哄簲鐢ㄥ競鍦洪噷鐨勫簲鐢ㄣ€?7. **鏈哄瀷/鍥轰欢姣斾笂娓告柊**锛歂E2210 + ColorOS 17锛孉BL 姣斾笂娓搁獙璇佺殑 NE2211/OOS16 鏂帮紱v4 宸查獙璇佽兘杩囷紝浣?ARB銆乻lot 琛屼负瑕佺暀鎰忋€?8. **鏈轰細鎴愭湰**锛氬悓鏍峰姏姘斾篃鍙互璧?OpenHarmony锛堝紑婧愮増锛夆€斺€旀棤闂簮/鏃犳縺娲伙紝浣嗘病鏈?AudioReach 鏂规銆佹病鏈?CamX锛岄┍鍔ㄨ浠庨浂锛屼笉瑙佸緱鏇寸渷銆?
---

## 9. 宸叉湁鍏堜緥锛堥兘涓嶇畻鎴愬姛锛?
- **`pawlickerr/hmos2negroni`**锛?026-10-26锛屼竴澶┿€丄I 鍗忓啓锛岄潪 fork 鑰屾槸 fork 鍚庢敼鍚嶏級锛氭敼浜?`00/11/12/30/32/38/42` 涓冧釜琛ヤ竵锛涘啓浜嗘暣鐩?`super.img`锛坄lib/lpimg.py`锛屽洜涓?negroni 娌℃湁 fastbootd 鍙互寤洪€昏緫鍒嗗尯"锛夈€乿4 `boot`銆乣fstab.negroni`锛坄/data=hmdata`锛夈€乣init.negroni.usb.cfg`锛坄a600000.usb`锛夈€丄dreno 730 鍥轰欢銆乵ainline msm + freedreno 鐨?Mesa銆?*娌℃湁寮€鏈鸿瘉鎹紝README 杩樻槸 sweet 鐨勶紝鎻忚堪鑷О slop**銆傚彲褰撴鏌ユ竻鍗曪紝涓嶈褰撴垚鏋溿€?- `SantiPT007/hmos2venus`锛堝皬绫?11 / SM8350锛夈€乣furangelz/hmos2lemonadep`锛堜竴鍔?9 Pro / SM8350锛夈€乣palazik/hmos5dodge`锛堜竴鍔?13 / SM8750锛夌瓑鍚屾棌 fork锛屽悓鏍锋病鏈夊叕寮€鐨勫紑鏈虹粨鏋溿€?- OpenHarmony 瀹樻柟/绀惧尯鐨勯獊榫欓€傞厤鍋滅暀鍦?**SD845 鏃朵唬**锛堝皬绫?6 / POCO F1 / 涓€鍔?6T锛夛紝**娌℃湁 SM8450**銆?
---

## 10. 鍙傝€?
- hmos2sweet锛歊EADME / `docs/ARCHITECTURE.md` / `docs/PATCHES.md` / `lib/builder.py` / `src/preinit/preinit.c`
- 浣犵殑 iOS 绉绘璁板綍锛歚1+10pro[ios]\README.zh-CN.md`锛坴4 / vbmeta / slot 浜嬫晠锛夈€乣GPU杩涘睍-2026-11-16.md`锛圸AP/PAS锛夈€乣WiFi杩涘睍-2026-11-16.md`锛圔HI/MHISTATUS锛?- negroni mainline锛歅MOS wiki銆乆EC 7.x `sm8450-oneplus-negroni.dts`銆乣sm8450-mainline/linux`
- 鍘傚晢婧愮爜锛歚OnePlusOSS/android_kernel_oneplus_sm8450`锛?.10锛夈€乣android_kernel_modules_and_devicetree_oneplus_sm8450`
- Mesa锛歠reedreno 鏂囨。銆乤7xx 绯诲垪 patch锛沗a730_sqe.fw` / `gmu_gen70000.bin` / `a730_zap.*`

---

## 11. 鐜板湪灏卞姩鎵嬬殑璇濓細M0 鏈€灏忔竻鍗?
1. **鐜**锛歐SL2 Ubuntu + Python3 + `e2fsprogs`/`unzip`/`curl`锛?5 GB 绌洪棽锛沜lone hmos2sweet锛沗tools/fetch_tools.sh`
2. **Donor**锛歚python3 fetchfw.py` 鎷?Pura 70 full OTA锛?.68 GB锛屽彲缁紶 + sha256锛?3. **鍐呮牳**锛歄nePlusOSS sm8450锛堜綘宸茶兘缂?SukiSU锛夆啋 鍏堝彧鎵?0001/0002/0003/0006/0009/0015锛岃烦杩?icnss 绯?鈫?鎵撳紑 EROFS/F2FS/EXT4/VFAT/DM/LOOP/USB_CONFIGFS/configfs/binder/`CONFIG_ANDROID_BINDERFS=n`
4. **preinit**锛氭敼 cmdline锛坄hardware=negroni`銆乣default_boot_device`銆乧onsole/earlycon 鍩哄潃瑕佹寜 waipio 鏌ワ級銆乣fstab.negroni`銆乻uper 鍒嗗尯琛ㄣ€佹ā鍧?insmod銆佹棩蹇楀垎鍖哄洖閫€
5. **浜ч暅鍍?*锛歚boot.img`(v4) + `vendor_boot`(DTB + 妯″潡) + `super.img`(LP 鍏冩暟鎹? + `vbmeta`(flags=3)
6. **鍒?*锛歚fastboot flash super/boot_a/vendor_boot_a/vbmeta_a` 鈫?`set_active a` 鈫?鎶?kmsg/hilog
7. **楠屾敹**锛氱湅鍒?`hmos-preinit: ...` 涓?HarmonyOS `init` 鐨?hilog 鍗?M0 閫氳繃锛涘啀杩涜 M1

> 濡傛灉 M0 缂栦笉杩?璧蜂笉鏉ワ紝鏈€澶氭崯澶?3鈥? 澶╁拰涓€涓彲鍥炲埛鐨勬満鍣?鈥斺€?杩欐槸鏁翠釜椤圭洰閲屾€т环姣旀渶楂樼殑涓€娆￠獙璇併€?
---

## 12. 鑳戒笉鑳藉仛鎴?DSU 鍖咃紵锛堣拷鍔狅級

**缁撹锛氶缚钂?5 涓嶈兘鍋氭垚 DSU 鍖呫€備綘鍚鐨?楦胯挋 DSU 鍖?鏄缚钂?4.0 GSI锛屾槸鍙︿竴鍥炰簨锛涜€屼笖杩欏彴鏈哄櫒杩炲畠閮借涓嶄笅銆?*

### 12.1 DSU 鍒板簳鎹簡浠€涔?
AOSP 鐨?DSU锛圖ynamic System Update锛孉ndroid 10+锛夛細鐢?root 鏉冮檺鎶?*涓€涓?system 闀滃儚**鍐欒繘 super 閲屾柊寤虹殑閫昏緫鍒嗗尯锛坄dsu` 缁勶級锛岄噸鍚椂鐢?*璁惧鍘熸潵鐨?boot / vendor / ramdisk** 寮曞锛屽彧鎶?`/system` 鎹㈡垚闀滃儚鍐呭锛涘啀閲嶅惎鍥炲埌鍘熺郴缁熴€?
鎵€浠?DSU 鐨勫墠鎻愶細闀滃儚蹇呴』鏄?**Android 鍏煎鐨?system 闀滃儚**锛岃€?**boot / kernel / vendor / ramdisk 涓€涓兘鎹笉浜?*銆?
### 12.2 涓轰粈涔堥缚钂?5 涓嶈锛堜笁鏉★紝浠讳竴鏉￠兘鑷村懡锛?
| # | 鍘熷洜 |
|---|---|
| 1 | hmos2sweet 鐨勬牳蹇冨氨鏄?*鎹㈡帀 ramdisk 閲岀殑 `/init`**锛坄hmos-preinit` 鏄犲皠 super銆佸啓 `/etc/fstab.required`銆佹寕 `/vendor`銆佸啀 exec 楦胯挋鐨?`init_early`锛夈€侱SU 涓嶇 ramdisk锛孉ndroid 鐨?init 鐓ф棫璺戯紝楦胯挋鐨?init 鏍规湰鎺ヤ笉涓娿€?|
| 2 | 楦胯挋 5 鐢ㄦ埛鎬佽閰?*鑷繁鐨?`/vendor`**锛圚DF 閰嶇疆銆丠DI 搴撱€乫stab銆乮nit cfg锛夈€侱SU 鍙粰 `/system`锛宍/vendor` 杩樻槸 ColorOS 鐨勶紝HDF host 涓€涓兘璧蜂笉鏉ャ€?|
| 3 | 鍐呮牳瑕佹墦 18 涓?OpenHarmony 琛ヤ竵锛坆inder 璇箟銆乤ccess_token銆乫s-verity銆乭mdfs 妯℃嫙銆丏RM master銆乵emcg鈥︼級銆侱SU 鐢ㄧ殑鏄澶囧師鍐呮牳锛屾病鏈夎繖浜涜ˉ涓侊紝楦胯挋鐢ㄦ埛鎬佺涓€姝ュ氨杩囦笉鍘汇€?|

### 12.3 "楦胯挋 DSU 鍖?鏄粈涔堬紙涓轰粈涔堝畠鑳借锛?
缃戜笂锛堝皬绫?绾㈢背/涓€鍔?8 Pro 閭ｆ壒鏁欑▼锛夎鐨?DSU 楦胯挋鍖咃紝鏄粠 **鍗庝负 nova 11锛堥獊榫?778G锛?* 鎶藉嚭鏉ョ殑 **HarmonyOS 4.0 system 闀滃儚**銆傞缚钂?4.x 鏈川浠嶆槸 **AOSP 搴曞瓙 + 鍗庝负鏈嶅姟**锛屾墍浠ュ畠鏄?Android 鍏煎闀滃儚"锛岃兘琚?DSU 褰?GSI 瑁呰繘 `/system` 璺戣捣鏉ャ€?
浣嗗畠锛?
- 涓嶆槸楦胯挋 5 / NEXT锛?*娌℃湁 OpenHarmony 杩愯鏃?*锛堝彧鑳借窇 Android 搴旂敤锛岃窇涓嶄簡 `.hap`锛夛紱
- 闇€瑕?*鍐呮牳瀹藉锛圫ELinux permissive锛? 鍏?AVB2.0** 鎵嶆湁杈冮珮鎴愬姛鐜囷紙鏁欑▼閲岀殑"鍐呮牳瀹藉鍗″埛鍖?灏辨槸骞茶繖涓級锛?- 渚濊禆绯荤粺閲屾湁 DSU 鏀寔锛圕olorOS 鍙兘宸茶鎺夛紝闇€瑕佸厛纭 `pm list packages | grep dynsystem`锛夈€?
### 12.4 杩欏彴鏈哄櫒杩樻湁涓€閬撳閲忕‖澧?
- super 10.50 GiB锛孋olorOS 鍗?9.92 GiB锛?*鍙墿 586.8 MiB**銆?- DSU 鐨?system 鍒嗗尯鍦?super 閲屾柊寤猴紝蹇呴』鏀惧緱涓嬫暣涓暅鍍忥細楦胯挋 4.0 GSI 绾?3鈥? GB锛岄缚钂?5 鐨?system 鏇村ぇ锛堟暣浣?7鈥? GB锛夈€?- 瑕?DSU锛屽氨蹇呴』鍏堝垹 `my_stock`(4.62G) / `odm`(1.29G) / `my_product`(1.22G) 杩欑被閫昏緫鍒嗗尯 鈥斺€?鍒犲畬 ColorOS 宸茬粡娈嬬己锛?鍏嶅埛鏈恒€侀殢鏃跺洖閫€"灏变笉鎴愮珛浜嗐€?
### 12.5 閭?浣庨闄╁厛璇曚竴涓?鎬庝箞鍋?
| 鏂规 | 鍋氭硶 | 浠ｄ环 | 鑳介獙璇佷粈涔?|
|---|---|---|---|
| **M0a 璇婃柇 boot锛堟帹鑽愶級** | 鍙埛 `boot_a`锛?.10 鍐呮牳 + 鏈€灏?preinit锛?*涓嶇 super**锛夛紱鍚姩鍚庢妸 UFS / DRM 杩炴帴鍣?/ kgsl / 瑙︽懜 / 鍥轰欢瀛樺湪鎬?/ 妯″潡鍔犺浇鍐欒繘 cache锛堣嫢瀛樺湪锛夋垨 pstore锛岀劧鍚庤嚜鍔ㄩ噸鍚?| 鍙敼 `boot_a` + vbmeta锛岄殢鏃跺埛鍥炲師鍘傦紱**鏁版嵁涓€鏉′笉涓?* | 鍐呮牳鑳藉惁鍦ㄨ繖鍙版満鍣ㄤ笂璺戙€乸reinit 鑳藉惁鎷垮埌鍒嗗尯銆佹樉绀?GPU 椹卞姩鍦ㄤ笉鍦ㄣ€佸埛鍐欓摼鏄惁鍙潬 |
| 楦胯挋 4.0 GSI + DSU | 鍏堝垹 super 閫昏緫鍒嗗尯鑵剧┖闂达紝鍐?DSU Sideloader + 4.0 GSI | 鐮村潖 ColorOS锛涜€屼笖涓嶆槸楦胯挋 5 | 鍙槸灏濋矞 4.0锛屽拰鏈」鐩洰鏍囨棤鍏?|
| 鍏ㄦ竻鍒峰叆锛堟璺級 | 鎸?搂7 浠?M0 寮€濮?| 鏁版嵁鍏ㄦ竻銆乻uper 閲嶅缓 | 鐪熸鐨勯缚钂?5 |

**M0a 灏辨槸"浣犳兂瑕佺殑閭ｇ DSU 寮忚瘯姘?**锛氭妸"鎹?system"鎹㈡垚"鍙崲 boot"锛岄獙瀹屽埛鍥烇紱瀹冩槸鍞竴闆舵暟鎹闄╁張鑳芥帹杩涢」鐩殑褰㈡€併€?
---

## 13. 鎵ц璁板綍锛?026-11-16 娣卞 鈫?11-17 鍑屾櫒锛?
### 13.1 鐜锛堝凡灏辩华锛?
| 椤?| 鐜扮姸 |
|---|---|
| WSL | Ubuntu-24.04锛? 鏍?/ 9 GB RAM / 941 GB 绌洪棽 |
| 宸ュ叿閾?| clang 18.1.3 + lld銆乬cc 13 + `aarch64-linux-gnu-gcc`銆乽nzip/brotli/lz4銆乸ython3+brotli銆乪rofs-utils 1.7锛圲buntu 鐗堬級 |
| 鍐呮牳婧愮爜 | `E:\1+10pro\_kernel\ksrc`锛氬畬鏁?Linux **5.10.236** GKI 鏍戯紙68,625 鏂囦欢 / 0.9 GB锛夛紝鍚?`build.config.msm.waipio` |
| 璁惧鍐呮牳閰嶇疆 | `E:\1+10pro\_kernel\device.config`锛堜粠璁惧 dump 鐨勭湡閰嶇疆锛?|
| 鍩虹嚎 boot | `kernelsu_patched_20260804_062211.img`锛歜oot header v4锛宬ernel 48,114,492 B锛宺amdisk 2,074,037 B锛屾枃浠?192 MB锛堝垎鍖?dump + AVB footer锛?|
| hmos2sweet | 宸?clone 鍒?`hmos2sweet2`锛?8.7 MB锛夊苟澶嶅埗鍒?WSL `/root/hmos2sweet` |
| donor OTA | 鍚庡彴涓嬭浇涓紙Pura 70 `update_full_base.zip`锛?,678,166,219 B锛宻ha256 `1c872454鈥锛孶RL 宸茬‘璁?200 OK锛?|
| 骞冲彴宸ュ叿 | `E:\1+10pro\_flashtool\{fastboot,adb}.exe` |

### 13.2 M0a 浜х墿锛堝凡鏋勫缓骞舵牎楠岋級

| 鏂囦欢 | 璇存槑 |
|---|---|
| `m0a/boot_m0a.img` | 48,443,392 B锛沨eader v4锛沰ernel **涓?SukiSU 闀滃儚閫愬瓧鑺傜浉鍚?*锛坰ha256 `a854051a鈥锛夛紱ramdisk = 鎴戜滑鐨勮瘖鏂?init锛?21,845 B gzip cpio锛夛紱sha256 `b3330b4d鈥 |
| `m0a/vbmeta_flags3.img` | 256 B锛宍Algorithm: NONE`銆乣Flags: 3`銆佹棤 descriptors |
| `m0a/diag_init.c` | 闈欐€?aarch64 璇婃柇 init锛圥ID 1锛?|
| `m0a/build_m0a.sh` / `verify_m0a.sh` | 鏋勫缓 / 鏍￠獙鑴氭湰锛圵SL锛?|
| `m0a/m0a.ps1` | `-Phase status\|backup\|flash\|collect\|restore` |
| `m0a/README-M0a.md` | 鍥涙鎿嶄綔鎵嬪唽 + 鍒よ + 鍥為€€ |
| `tools/bootimg4.py` | boot header v3/v4 瑙ｅ寘/鎵撳寘锛堝彧鏀?`ramdisk_size` 涓?cmdline锛屼繚鐣?1584 瀛楄妭鍘熷ご锛?|

### 13.3 M0a 鏈熼棿鍙戠幇鐨勪袱涓‖浜嬪疄

1. **鍐呮牳娌″紑 devtmpfs**锛坄# CONFIG_DEVTMPFS is not set`锛夈€?   Android 闈?ueventd 寤?`/dev`锛屾墍浠ユ垜浠殑 init 蹇呴』鑷甫涓€涓?mini-ueventd锛氭壂 `/sys/class/block/*/uevent` 鎷?`MAJOR/MINOR/DEVNAME/PARTNAME`锛岃嚜宸?`mknod` 鍑?`/dev/block/*` 涓?`/dev/block/by-name/*`锛屽啀缁?input/drm/misc/graphics 寤鸿妭鐐广€?*宸插湪 `diag_init.c` 瀹炵幇骞剁紪璇戣繘鍘?*锛堣繖涔熺洿鎺ユ槸 M0b preinit 瑕佺敤鐨勪唬鐮侊級銆?2. **ramoops 瀛樺湪**锛坄vendor_boot.img` 鐨?DTB 閲屾湁 `ramoops`/`pstore` 瀛楁牱锛宍CONFIG_PSTORE_RAM=y`銆乣CONFIG_PSTORE_CONSOLE=y`锛夛紝涓?`CONFIG_PANIC_TIMEOUT=-1`锛坧anic 涓嶈嚜鍔ㄩ噸鍚紝鏃ュ織鐣欏緱浣忥級銆傝瘖鏂?init 鐨勬棩蹇楄蛋 `/dev/kmsg` 鈫?ramoops锛岃瘖鏂畬鑷姩 `reboot bootloader`锛屽啀杩?TWRP 浠?`/sys/fs/pstore` 鍙栥€?
### 13.4 M0a 楠屾敹鏍囧噯

`logs\m0a-*.log` 閲屽嚭鐜帮細`pid=1 uid=0`銆乣ueventd: N block nodes, M by-name links`銆乣by-name total 鈥锛堝惈 super/boot_a/vbmeta_a锛夈€?`super: LP geometry magic 0x616c4467`锛圠P 鍏冩暟鎹彲璇?= M0b 纭墠鎻愶級銆乣DSI-1 鈥?status=connected`銆乣/dev/kgsl-3d0: present`銆乣/dev/input/event*`銆乸store 鎸傝浇鎴愬姛銆?
### 13.5 M0b 杩涘睍锛堜唬鐮佸凡灏变綅锛岀瓑 OTA锛?
| 椤?| 鐘舵€?|
|---|---|
| `m0b/preinit/preinit.c` | 宸插啓骞剁紪璇戦€氳繃锛堥潤鎬?aarch64锛?73 KB锛夛細mini-ueventd 鈫?super LP 瑙ｆ瀽 鈫?dm-linear 鈫?`/etc/fstab.required` 鈫?exec `/bin/init_early`锛涙棩蹇楄蛋 `/dev/kmsg`锛坮amoops锛? cache 鍥為€€ |
| `m0b/build_ramdisk.py` | 瑙?HarmonyOS `HARMONY!` ramdisk锛坓zip newc cpio锛夛紝鎶?preinit 瑁呮垚 `/init`锛屼繚鐣?`/bin/init_early` |
| `m0b/ota_inspect.py` | 澶嶇敤 hmos2sweet 鐨?`update.bin` 瑙ｆ瀽鍣細鍒楃粍浠讹紙鍚?sha256锛夈€佺畻 super 瀹归噺銆佹娊鍙栭暅鍍?|
| `m0b/build_m0b.sh` | 涓€閿細鏍￠獙 sha256 鈫?鍒楃粍浠?鈫?鎶介暅鍍?鈫?寤?ramdisk 鈫?鎵?boot v4 |
| EROFS 閲嶅缓 | **楠岃瘉閫氳繃**锛歚fsck.erofs --extract` + `mkfs.erofs -zlz4hc` 寰€杩斿悗 uid/gid/鏉冮檺/杞摼鍏ㄩ儴淇濈暀锛圫ELinux 鏍囩浼氫涪锛屼絾 `hmos.permissive=1` 涓嬫棤鎵€璋擄級銆傚洜姝ゆ敼 vendor.img / 鍔?HAL 涓嶉渶瑕?Android 鐗?erofs 宸ュ叿閾?|
| donor OTA | 涓嬭浇涓紙4.2 / 5.68 GB锛岀害 1.2 MB/s锛岃繕闇€ ~20 鍒嗛挓锛?|

M0b 鐨勪笅涓€涓插姩浣滐紙OTA 钀藉湴鍚庤嚜鍔ㄦ墽琛岋級锛?1. 鏍￠獙鏁村寘 sha256锛坄1c872454鈥锛?2. 鍒楃粍浠?鈫?纭 system/vendor/sys_prod/chip_prod/cust/patch/version/preload/ramdisk 閮藉湪锛岀畻 `super` 浣欓噺
3. 鎶介暅鍍?鈫?鐪?vendor.img 閲岀幇鎴愮殑 `fstab.*`锛堣嫢鍙鐢?Kirin 鐨勫悕瀛楋紝M0b 杩?vendor 閮戒笉鐢ㄩ噸寤猴級
4. 寤?ramdisk + `boot_m0b.img`锛堝厛鐢ㄥ悓涓€涓?SukiSU 鍐呮牳锛岄獙璇?鑳?mapping + 鑳借捣 init"锛?5. 鍐嶆崲"鎵撲簡 OHOS 琛ヤ竵鐨勮嚜缂?5.10 鍐呮牳"锛圡0c锛?
### 13.6 OTA 宸茶惤鍦帮紙2026-11-17 00:11锛?
**`sha256 = 1c872454021bc9c2479ee3f09ea6d96e4b8631119606468d1fa6af0a52d189d1`锛?,678,166,219 B锛屼笌 catalog 瀹屽叏涓€鑷淬€?*

缁勪欢瀹炴祴锛?4 涓紝`update.bin` 6.33 GiB锛夛細

| 缁勪欢 | 澶у皬 | 璇存槑 |
|---|---|---|
| `system` | 2.9 GB | EROFS锛宍/bin` 鏄蒋閾撅紙system-as-root 鈫?`/usr`锛?|
| `sys_prod` | 1.7 GB | EROFS |
| `vendor` | 652.6 MB | EROFS锛屽惈 **`/etc/fstab.Kirin9010E`**锛坔ardware 鍚?= `Kirin9010E`锛?|
| `chip_prod` | 552.4 MB | EROFS |
| `cust` | 5.9 MB | EROFS |
| `patch` | 20 KB | EROFS |
| `ramdisk` | 3.0 MB | `HARMONY!` 瀹瑰櫒 + gzip newc cpio锛?49 椤癸紱`init` 鏄寚鍚?`bin/init_early` 鐨勮蒋閾撅紝`bin/init_early` 156 KB ELF |
| ~~`version` / `preload`~~ | 鈥?| **杩欎釜鍖呮病鏈?*锛坧reinit 鐨?fstab 浼氳嚜鍔ㄨ烦杩?nofail 椤癸級 |

**瀹归噺缁撹锛欻armonyOS 渚у彧鍗?5.8 GB锛宻uper 10.5 GiB 鈫?浣欓噺 4.7 GB锛?5%锛夈€?* 姣旈浼板鏉惧緱澶氾紝鑰屼笖鍙 6 涓€昏緫鍒嗗尯銆?
宸茬敓鎴愶細

| 鏂囦欢 | 鍐呭 |
|---|---|
| `m0b/ramdisk.gz` | 2,835,637 B锛汷TA 鐨?249 椤?cpio 鍏ㄩ儴淇濈暀锛宍/init` 鎹㈡垚鎴戜滑鐨?preinit锛?73,184 B锛夛紝`/bin/init_early` 鍘熸牱 |
| `m0b/boot_m0b.img` | 50,958,336 B锛沨eader v4锛沰ernel 浠嶆槸 SukiSU 鐨勶紙閫愬瓧鑺傜浉鍚岋級锛泂ha256 `2e592f5b鈥 |
| `/root/hmos/ota/*.img` | 7 涓垎鍖洪暅鍍忓凡鎶藉嚭锛坰ystem/vendor/sys_prod/chip_prod/cust/patch/ramdisk锛?|

### 13.7 M0b 鐨勪笅涓€涓矓璺彛锛圫ELinux锛?
- 鍘傚晢鍐呮牳鐨?SELinux 鍙互闈?cmdline `enforcing=0` 璧锋垚 permissive锛屼絾 **OHOS init 鑷繁浼氭妸 `enforce` 鍐欏洖 1**锛坔mos2sweet 涓撻棬鎵撲簡 `hmos.permissive=1` 琛ヤ竵鏉ュ帇浣忓畠锛夈€?- 鐢?`selinux=0` 瀹屽叏鍏虫帀 SELinux 鍙互鍏堣瘯锛氳兘璧峰氨璇存槑鍙樊杩欎釜琛ヤ竵锛涜捣涓嶆潵灏辫鍏堝仛 **M0c锛氳嚜缂?5.10 GKI 鍐呮牳**锛坄ksrc` 灏卞湪鏈満锛屽伐鍏烽摼 clang 18 宸茶锛岄渶琛?OpenHarmony 鐨?0001/0002/0003 绛夎ˉ涓侊級銆?
### 13.8 M0 鏃堕棿琛ㄦ洿鏂帮紙瀹炴祴锛?
| 閲岀▼纰?| 鐘舵€?|
|---|---|
| M0a 浜х墿 | 鉁?瀹屾垚锛堢瓑鎵嬫満杩炰笂鍗冲彲璺戯級 |
| M0b 浜х墿锛坮amdisk + boot + 闀滃儚锛?| 鉁?瀹屾垚 |
| M0c 鑷紪鍐呮牳锛圤HOS 琛ヤ竵锛?| 鈴?鎺ヤ笅鏉ワ紙棰勮 3鈥? 灏忔椂鏋勫缓杩唬锛?|
| 鐪熸満棣栧埛锛堟竻 super + 鍐欏叆 6 涓垎鍖猴級 | 鈴?闇€瑕佹墜鏈鸿繛鎺ヤ笌娓呮暟鎹鍙?|

### 13.9 M0b 闀滃儚闆嗗凡鍏ㄩ儴灏辩华锛?026-11-17 00:20锛?
| 鏂囦欢 | 璇存槑 |
|---|---|
| `m0b/boot_m0b.img` | 50,958,336 B锛沰ernel = SukiSU锛堟湭鍙橈級锛況amdisk = OTA 249 椤?+ 鎴戜滑鐨?preinit锛沜mdline `hardware=negroni 鈥?selinux=0 enforcing=0`锛泂ha256 `7bfffb6c鈥 |
| `m0b/vendor_negroni.img` | 673,529,856 B锛汬armonyOS vendor + 鏂板 `/etc/fstab.negroni`銆乣/etc/init.negroni.cfg`锛沗mkfs.erofs -zlz4hc` 閲嶅缓锛宍fsck.erofs` 閫氳繃锛泂ha256 `d3c42b62鈥 |
| `m0b/ota/*.img` | system 2.9G / sys_prod 1.7G / chip_prod 552M / cust 5.9M / patch 20K锛堝師鏍凤紝鏈敼锛?|
| `m0b/m0b.ps1` | `status\|backup\|flash\|collect\|rollback`锛沗flash` 闇€瑕佽緭鍏?YES |

**鍙垹 ColorOS 鐨?`system` / `vendor` / `my_stock` 涓変釜閫昏緫鍒嗗尯**锛堝叾瀹?`my_*`銆乣product`銆乣odm`銆乣system_ext`銆乣*_dlkm` 鍏ㄩ儴淇濈暀锛夛紝
鎵€浠ュ洖婊?= 鐢?`-Phase rollback -ImagesDir <ColorOS 鍖?/images` 鎶?`system.img`銆乣vendor.img`銆乣my_stock.img` 鍒峰洖鍘?+ 鎭㈠ boot/vbmeta銆?瀹归噺锛欻armonyOS 6 涓垎鍖?5.78 GiB + ColorOS 鍓╀綑绾?3.15 GiB = 8.93 GiB < 10.50 GiB銆?
### 13.10 杩欒疆鎸栧嚭鏉ョ殑鍏抽敭浜嬪疄

1. **`/system/system/etc/init.cfg` 閲屽啓姝讳簡涓や欢纭欢鐩稿叧鐨勪簨**锛?   `import "/vendor/etc/init.${ohos.boot.hardware}.cfg"` 鍜?`mount_fstab /vendor/etc/fstab.${ohos.boot.hardware}`銆?   鈫?鎵€浠?`hardware=negroni` 蹇呴』閰?`init.negroni.cfg` + `fstab.negroni`锛屼袱涓枃浠堕兘宸插仛濂斤紱缂轰换浣曚竴涓?init 浼氭姤閿欍€?2. **鍘熷巶 `fstab.Kirin9010E` 閲?`/data` 鐨勭被鍨嬫槸 `hmfs`**锛堝崕涓烘枃浠剁郴缁燂級鈥斺€旀墍浠?OHOS 5.1 鐨勬暟鎹垎鍖轰笉鏄櫘閫?f2fs锛?   hmos2sweet 鍦?sweet 涓婄敤鐨勬槸 `f2fs` 骞朵笖鑳借窇锛岃鏄?hmfs 椹卞姩鎺ュ彈 f2fs 鏍煎紡鍖栵紝M0b 鍏堟寜 f2fs 璧般€?3. **boot.img 鐨?ramdisk 鏄?LZ4 鍘嬬缉**锛坢agic `0x184C2102`锛夛紝涓嶆槸 gzip锛涘唴鏍搁厤缃噷 `RD_LZ4=y`銆乣RD_GZIP=y` 閮芥湁锛屾垜浠殑 gzip cpio 鐓ф牱鑳借В銆?4. 鍖呴噷鏈?`init_boot.img`锛? MB锛夛紝浣?SukiSU 鏄€氳繃鏀?`boot.img` 鐨?ramdisk 鐢熸晥鐨?鈫?**瀹為檯浣跨敤鐨勬槸 boot.img 鐨?ramdisk**锛圡0a 涓婃満鍗冲彲鏈€缁堢‘璁わ級銆?5. 鍘熷巶 vendor 閲屾湁 `cfg_*.ini`锛堟寜鏈哄瀷涓婄櫨涓級鍜?`fstab.Kirin9010E` 鈥斺€?璇存槑 HarmonyOS 鐨勭‖浠舵娊璞＄‘瀹炴槸"鎸夊瀷鍙峰悕閫夐厤缃?锛宍hardware=` 鏄叧閿紑鍏炽€?
### 13.11 鐢ㄦ埛纭锛氬鐢ㄦ満銆佹棤鏁版嵁锛?026-11-17 00:35锛?
鈫?M0b 鐨勬竻鏁版嵁闂ㄦ娑堝け锛屾祦绋嬬畝鍖栦负**涓€鏉″懡浠?*锛坄m0/run_m0.ps1 -Phase start`锛夛細
M0a 璇婃柇鍒峰叆锛堜笉纰?super锛夆啋 鑷姩杩?TWRP 鍙?pstore 鈫?闂竴鍙?YES 鈫?M0b 娓?super 瑁呴缚钂欍€?
### 13.12 M0c 鍐呮牳鏋勫缓锛堣繘琛屼腑锛?
| 椤?| 鐘舵€?|
|---|---|
| GKI common 婧愮爜 | 宸蹭粠 `_kernel\ksrc` 鎵撴垚 `ksrc.tar`锛?010 MB锛學indows 涓?tar 鐢ㄤ簡 11.6 鍒嗛挓锛夆啋 WSL `/root/kernel` 瑙ｅ嚭 1.2 GB |
| 绗竴娆℃瀯寤哄け璐?| `./scripts/*.sh: Permission denied`锛圵indows tar 涓簡鍙墽琛屼綅锛? `unrecognized assembler`锛涘凡鍔?`chmod` 淇锛岄噸璺戜腑 |
| 閰嶇疆 | 鐩存帴鐢ㄦ墜鏈?dump 鍑烘潵鐨?`device.config`锛?.10.236锛孡TO_CLANG_FULL + CFI_CLANG锛?|
| 宸ュ叿閾?| clang 18.1.3 + lld锛圲buntu锛夛紝`ARCH=arm64 LLVM=1 -j8` |
| OnePlusOSS vendor 鏍?| 鉁?宸?clone `oneplus/sm8450_b_16.0_oneplus_10_pro`锛?.4 GB / 68,698 鏂囦欢锛宍/root/op-kernel`锛夆€斺€斿惈 techpack銆丏TS锛屾槸 M1 鏄剧ず/闊抽 HAL 涓?vendor 妯″潡鐨勬簮鐮佸熀纭€ |

### 13.13 鑷紪鍐呮牳鎵撻€氾紙2026-11-17 03:40锛?
**绗竴娆℃垚鍔熺紪鍑?`Image`锛?1,804,288 B锛宻ha256 `f53891dd鈥锛屽唴寤?initramfs锛堝帇缂?cpio 閲岀‘璁ゅ惈 `/init`锛夈€?*

涓€璺俯鐨勫潙涓庝慨娉曪紙閮藉凡鑴氭湰鍖栧湪 `m0c/`锛夛細

| 闂 | 鍘熷洜 | 淇硶 |
|---|---|---|
| `can't open file kernel/oplus_cpu/sched/Kconfig` 脳27 | 杩欐５鏍戦噷 38 涓鍙烽摼鎺ユ寚鍚戞湭闅忔簮鐮佸彂甯冪殑 `vendor/oplus` 浠撳簱 | 鏂摼鏇挎崲涓虹┖鐩綍 + Kconfig stub锛坄fix_symlinks.sh`銆乣stub_loop.sh`锛?|
| 閰嶇疆鍙湁 1946 琛屻€乣MODULES`/`EROFS` 鍏ㄤ涪 | `device.config` 鏄?**UTF-16LE**锛宬config 璇讳笉鎳?| `iconv` 杞?UTF-8锛坄convert_config.sh`锛?|
| `register 'sp' unsuitable` | 鍙粰 `LLVM=1` 娌＄粰 `CROSS_COMPILE`锛宑lang 鎸?x86 缂?arm64 澶?| `ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LLVM=1` |
| `scan_adjusted` 鏈０鏄?| 鍘傚晢琛ヤ竵缂轰簡澹版槑 | 琛ュ０鏄?|
| `proportional_reclaim` 鏈娇鐢?鈫?forbidden warning | 鐢ㄥ埌瀹冪殑鍘傚晢浠ｇ爜涓嶅湪 | `__maybe_unused` + `CONFIG_WERROR=n` |
| clang 18 姣?clang 12 涓?| `-Wstrict-prototypes` 绛夋柊榛樿 | 涓€涓?`-Wno-*`锛坄build12~18.sh`锛?|
| `debug_kinfo.c` 鐢?`offsetof(struct module)` | `MODULES=n` 鏃?`struct module` 涓嶅畬鏁?| 鍏?`DEBUG_KINFO` |
| `duplicate symbol __tracepoint_ufshcd_*` | `ufshcd.c` 涓?`ufs-qcom.c` 閮?`CREATE_TRACE_POINTS` | 鍘绘帀 ufs-qcom 鐨勯偅浠?|
| `undefined register_device_proc / qcom_clk_set_flags` | UFS 椹卞姩琚竴鍔犳敼杩囷紝璋冪敤 `vendor/oplus` 绉佹湁鍑芥暟 | **鍏堝叧鎺?UFS**锛堟湰杞笉闇€瑕佸瓨鍌級 |
| `undefined kvzalloc / mmget_still_valid / kvmalloc` | 杩欐５鏍戞妸 `kvmalloc` 澹版槑鎴?extern銆乥inder 鐢?5.9 宸插垹鐨?API | 鏀硅皟 `kvmalloc_node`銆佸垹 `mmget_still_valid` 妫€鏌?|

褰撳墠閰嶇疆瑕佺偣锛歚INITRAMFS_SOURCE=/root/m0a/v4/initramfs`锛堝唴寤?`/init`锛夈€乣DEVTMPFS=y`銆乣EROFS/F2FS/DM/PSTORE=y`銆乣MODULES=n`銆乣LTO/CFI=n`銆乁FS 鍏抽棴銆?
浜х墿锛歚m0a/boot_m0c.img`锛?2,160,128 B锛屽唴鏍?+ v4 ramdisk 鍙屼繚闄╋紝sha256 `0360c0c8鈥锛夈€?鍒ゆ嵁锛氬埛鍏ュ悗鎵嬫満浼?*鑷姩鍏虫満**锛坴4 init 鐨勬渶鍚庝竴姝ユ槸 `RB_POWER_OFF`锛? 鎴戜滑鐨?`/init` 鐪熺殑浠?PID 1 璺戣捣鏉ヤ簡銆?
### 13.14 閲岀▼纰戯細鎴戜滑鐨?`/init` 鍦ㄧ湡鏈轰笂浠?PID 1 杩愯 鉁咃紙2026-11-17 12:30锛?
鍒峰叆"鑷紪鍐呮牳锛堝唴寤?initramfs锛? v4 璇婃柇 init"鍚?*鎵嬫満鍏虫満** 鈥斺€?`RB_POWER_OFF` 鏄?v4 init 鐨勬渶鍚庝竴鏉℃寚浠わ紝杩欐槸**纭畾鎬ц瘉鎹?*銆侻0a 鐨勬牳蹇冮棶棰樿В鍐炽€?
### 13.15 UFS 涓轰粈涔堣捣涓嶆潵锛堝凡瀹氫綅锛?
`logdump` 涓€鐩翠负绌猴紝浣嗗師鍥犱笉鏄?init 娌¤窇锛?*杩欓 GKI 鍐呮牳閲屾病鏈変换浣曞钩鍙伴┍鍔?*銆傚姣?`arch/arm64/configs/vendor/waipio_GKI.config`锛堝巶鍟?fragment锛?42 琛岋紝鍏朵腑 **423 涓?`=m`**锛夛細

| 闇€瑕?| 鎴戜滑鍐呮牳閲?| 鐪熸満涓?|
|---|---|---|
| `QCOM_CLK_RPMH`锛堟椂閽燂級 | 鉂?鏃?| 妯″潡 |
| `QCOM_RPMH` / `REGULATOR_RPMH` | 鉂?鏃?| 妯″潡 |
| `SPMI_MSM_PMIC_ARB`銆乣MFD_SPMI_PMIC` | 鉂?鏃?| 妯″潡 |
| `ARM_SMMU`锛圛OMMU锛?| 鉂?鏃?| 妯″潡 |
| `INTERCONNECT_QCOM_WAIPIO`锛圛CC锛?| 鉂?鏃?| 妯″潡 |
| `PINCTRL_WAIPIO` | 鉂?鏃?| 妯″潡 |
| `QCOM_COMMAND_DB` | 鉂?鏃?| 妯″潡 |
| `SCSI_UFS_QCOM` + `PHY_QCOM_UFS` | 鉁?宸插唴寤?| 妯″潡 |

缁撹锛?*UFS 鎺㈡祴蹇呯劧澶辫触**锛堟病鏈夋椂閽?绋冲帇鍣?IOMMU/ICC锛夛紝鎵€浠?init 铏界劧璺戜簡锛屼絾鐪嬩笉鍒颁换浣曞潡璁惧锛屽啓涓嶈繘 `logdump`銆?
**涓嬩竴闃舵鏂规锛圓ndroid 鍋氭硶锛?*锛氬唴鏍?`MODULES=y` + 鍚堝苟 `waipio_GKI.config` fragment锛屾妸 423 涓ā鍧楄繛鍚?`modules.load` 涓€璧锋斁杩涘唴寤?initramfs锛岀敱 preinit 鎸夊簭 `finit_module` 鈥斺€?杩欎篃姝ｆ槸鍚庨潰鏄剧ず/GPU/闊抽鎵€闇€瑕佺殑鍚屼竴濂楁満鍒躲€?
### 13.16 妯″潡鍖栧唴鏍稿崱鍦ㄥ摢锛?026-11-17 13:30锛?
鍚堝苟 fragment 鍚庨厤缃甯革紙`=y` 1946 / `=m` 341锛屽惈 `SCSI_UFS_QCOM=m`銆乣QCOM_CLK_RPMH=m`銆乣ARM_SMMU=m`銆乣REGULATOR_RPMH=m`锛夛紝浣嗙紪璇戞毚闇蹭簡 **OSS 鏍戜笌鏈彂甯冪殑 `vendor/oplus` 娣卞害鑰﹀悎**锛?
| 澶辫触鐐?| 缂虹殑涓滆タ |
|---|---|
| `drivers/soc/qcom/fsa4480-i2c.c` | `oplus_audio_switch.h` |
| `drivers/gpu/msm/kgsl.c` | `vendor/oplus/.../reserve_area.h` |
| `drivers/tty/serial/msm_geni_serial.c` | `serial_trace.h`锛坱race include 璺緞闂锛?|
| `drivers/soc/qcom/icnss2/main.o` | `soc/oplus/boot/boot_mode.h` |
| `drivers/dma-buf/heaps/qcom_system_heap.c` | `oplus_boostpool/oplus_boost_pool.h` |
| `drivers/soc/qcom/*` 澶氬 | `soc/oplus/boot/oplus_project.h` |
| `drivers/clk/qcom/clk-debug.c` / `hyp_core_ctl.c` | 鐢熸垚寮?trace 澶?|

宸茶嚜鍔ㄥ叧鎺変竴鎵癸紙`QCOM_DCVS`銆乣ICNSS2`銆乣QCOM_KGSL`銆乣AW8697_HAPTIC`銆乣LIBNVDIMM`銆乣SM_DEBUGCC_*`鈥︼級銆?*鍓╀綑闇€瑕佸仛閫夋嫨**锛?
- **A锛堟帹鑽愶級鏈€灏忓唴鏍?*锛歚MODULES=n` + 鎵嬪伐鎵撳紑绾?30 涓瓨鍌ㄩ摼绗﹀彿锛圲FS/UFSHCD/UFS_QCOM銆丳HY_QCOM_UFS*銆丵COM_CLK_RPMH銆丷EGULATOR_RPMH銆丼PMI銆丳INCTRL_WAIPIO銆丄RM_SMMU銆両NTERCONNECT_QCOM_WAIPIO銆丵COM_COMMAND_DB銆丵COM_SMEM/RPMH锛夛紝瀹屽叏閬垮紑鍘傚晢鑰﹀悎鐨勯┍鍔ㄣ€?- **B** 缁欑己澶辩殑 vendor 澶存枃浠跺啓 stub锛岀‖缂栨ā鍧楀寲鍐呮牳锛堝伐浣滈噺澶с€佸悗缁瘡涓┍鍔ㄩ兘瑕?stub锛夈€?- **C** 鍥炲埌"stock 鍐呮牳 + 鎶?init 鏀捐繘 vendor_boot 鐨?vendor ramdisk"杩欐潯**鍞竴娌¤瘯杩?*鐨勮矾锛堟垚鏈渶浣庯紝浣嗘湭鐭ワ級銆?
### 13.17 璁惧鐘舵€?
`boot_a` 宸叉槸瀹樻柟闀滃儚銆乣misc` 宸叉竻銆乻lot a 宸叉縺娲伙紝浣嗘墜鏈哄綋鍓?*涓嶆灇涓?*锛堟棤 adb/fastboot/USB锛夆€斺€?杩炵画鏁板皬鏃剁殑閲嶅惎寰幆寰堝彲鑳芥妸鐢佃€楀敖浜嗐€傞渶瑕侊細鍏呯數 10鈥?5 鍒嗛挓 鈫?闀挎寜鐢垫簮 15 绉?鈫?`闊抽噺涓?鐢垫簮` 杩?fastboot銆?
### 13.18 绗笁鏉¤矾锛氭妸 init 鏀捐繘 vendor_boot 鐨?vendor ramdisk锛?026-11-17 18:16锛?
鍘熺悊锛氬唴鏍镐緷娆¤В鍖?generic ramdisk锛圓ndroid 鐨?init锛夊拰 vendor ramdisk锛屽悗瑙ｅ寘鐨?`/init` 瑕嗙洊鍓嶈€咃紱瀹樻柟鍐呮牳鏈夊叏閮ㄩ┍鍔紝preinit 鍙嚜宸卞姞杞?`/lib/modules/modules.load` 閲岀殑 461 涓巶鍟嗘ā鍧椼€?
- 宸ュ叿锛歚m0c/repack_vendor_boot.py` 鈥斺€?淇埌**閫愬瓧鑺傝繕鍘熷師鍘傞暅鍍?*锛堣嚜妫€ `identical=True`锛夛紱鐪熷疄甯冨眬鏄?`ramdisk | dtb | bootconfig | pad | table`锛堜笌 AOSP 鏂囨。涓嶅悓锛?- 浜х墿锛歚m0c/vendor_boot_hmod.img`锛?92 MB锛涘師鍘?vendor ramdisk 497 椤?+ 鎴戜滑鐨?preinit 浣?`/init` + HarmonyOS ramdisk 246 椤癸紝鑷鍥涢」鍏ㄨ繃锛?- **缁撴灉锛氬埛鍏ュ悗 ABL 鍥為€€鍒?fastboot锛屾病璧锋潵**锛涘凡鎭㈠鍘熷巶 `vendor_boot_a`/`boot_a`/`vbmeta_a` 骞舵竻 `misc`銆傚師鍥犳湭鐭ワ紙娌℃湁鏃ュ織閫氶亾锛夛紝杩欐潯璺殏涓嶆帹杩涖€?
### 13.19 闀挎湡姝ｈ矾锛歀ineageOS sm8450 鍐呮牳锛堝凡 clone锛?
璁惧骞冲彴浠ｅ彿 **`wly`**銆侺ineageOS 鏈夋暣濂楀彲缂栬瘧婧愮爜锛?
| 浠撳簱 | 鍒嗘敮 | 澶у皬 |
|---|---|---|
| `LineageOS/android_kernel_oneplus_sm8450` | lineage-22.2 | 1.4 GB 鉁?|
| `LineageOS/android_kernel_qcom_sm8450-modules` | lineage-20 | 281 MB 鉁?|
| `LineageOS/android_kernel_qcom_sm8450-devicetrees` | lineage-20 | 36 MB 鉁?|

杩欐槸鍞竴鑳藉湪 OSS 鏉′欢涓嬬紪鍑?*瀹屾暣椹卞姩**锛圲FS/鏄剧ず/GPU/闊抽锛変笖涓嶅甫 `vendor/oplus` 缂轰欢鐨勫簳搴с€備笅涓€姝ワ細鎸?AOSP prebuilts 娴佺▼缂栧唴鏍?妯″潡 鈫?鎵?OpenHarmony 琛ヤ竵锛坅ccess_token銆乥inder ioctl銆乫s-verity stub銆乭mdfs銆丏RM master銆乵emcg銆乭mos.permissive锛夆啋 鐢熸垚 boot + 妯″潡 ramdisk 鈫?娓?super 瑁呴缚钂欍€?
### 13.20 vendor_boot 閲嶆墦鍖呯殑鐪熺浉锛?026-11-17 18:40锛?
鐪熷疄甯冨眬锛?*涓?AOSP 鏂囨。涓嶅悓**锛夛細

```
header(2128) | vendor ramdisk | dtb | bootconfig | pad | table |
pad | 绗簩浠?bootconfig 鏂囨湰 | pad(0x2000) | AVB vbmeta | 鈥?| AVB footer(鏈€鍚?64B)
```

涓や釜鑷村懡閿欒锛堥兘淇簡锛夛細

| 閿欒 | 鍚庢灉 | 淇硶 |
|---|---|---|
| 涓㈡帀 **AVB footer + vbmeta blob** | ABL 鐩存帴鎷掔粷闀滃儚 鈫?鎺?fastboot | 浠庡師闀滃儚灏鹃儴澶嶅埗锛宖ooter 閲?`original_image_size`/`vbmeta_offset` 璺熸柊甯冨眬鏀瑰啓 |
| v4 澶村瓧娈靛綋 u64 瑙ｆ瀽 | `vendor_ramdisk_size` 璇婚敊 鈫?鍏ㄧ洏閿欎綅 | vendor_boot v4 鍦板潃鏄?**u32**锛宍vendor_ramdisk_size` 鍦ㄥ亸绉?**24** |

鐜板湪 `repack_vendor_boot.py --self-test` 杈撳嚭 **`identical=True`**锛堝師鏍烽噸寤洪€愬瓧鑺傜浉鍚岋級锛屾敼 ramdisk 鍚庤嚜妫€ `avb=True`銆?
### 13.21 misc / bootloader_control 鐨勬纭粨鏋勶紙2026-11-17 18:50锛?
涔嬪墠鎵嬪啓 `bootloader_control` 琚?ABL 鎷掔粷鐨勬牴鍥犳壘鍒颁簡锛?*缁撴瀯浣撲互 `slot_suffix` 寮€澶达紝涓嶆槸浠?magic 寮€澶?*锛?
| 鍋忕Щ | 瀛楁 |
|---|---|
| 0 | `slot_suffix[4]` = `"_a\0\0"` |
| 4 | magic `0x42414C54`锛堝皬绔瓧鑺?`54 4c 41 42` = "TLAB"锛?|
| 8/9/10/11 | version=1 / nb_slot=2 / recovery_tries / merge_status |
| 14 | `slot_info[4]`锛堟瘡椤?2 瀛楄妭锛歱riority:4 tries:3 successful:1 verity:1锛?|
| 30 | `crc32_le`锛堝 0..29 璁＄畻锛?|

宸ュ叿锛歚tools/gen_misc.py` 鈫?鐢熸垚 `m0c/misc_good.img`锛? MB锛宑rc `0x9e60bfab`锛岃嚜妫€ OK锛夈€傝繖鏍?`fastboot flash misc misc_good.img` 灏辫兘鎭㈠妲戒綅鍏冩暟鎹紝涓嶅啀渚濊禆鏁村寘绾垮埛銆?
### 13.22 浜屽垎缁撴灉锛?026-11-17 20:35锛夆€斺€斿唴鏍歌兘鍚姩锛孶FS 杩樺樊 ICC

涓€鍔?OSS 鏍戯紙`/root/op-kernel`锛夋寜涓嬮潰閰嶇疆瀹炴祴锛?
| 鐗堟湰 | 閰嶇疆 | 鐪熸満缁撴灉 |
|---|---|---|
| v4锛?2:30锛?| 鍘熷巶 GKI 閰嶇疆锛屾棤瀛樺偍閾?| 鉁?**鍚姩 + init 杩愯 + 鑷姩鍏虫満**锛圡0a 鏍稿績宸茶瘉瀹烇級 |
| v10锛?0:11锛?| + SMMU/IOMMU/ICC/UFS/鏃堕挓/绋冲帇鍣?| 鉂?panic 鏃犻檺閲嶅惎 |
| v11锛?0:24锛?| 鍘绘帀 SMMU/IOMMU/ICC | 鉁?鍚姩 + 鍏虫満锛坄logdump` 绌?= UFS 鏈捣锛?|
| v12锛?0:31锛?| 鍔犲洖 SMMU/IOMMU + `iommu.passthrough=1` | 鉁?鍚姩 + 鍏虫満锛坄logdump` 浠嶇┖锛?|

**缁撹**锛?1. **SMMU/IOMMU 鏈韩涓嶆槸 panic 婧?*锛坴12 甯?SMMU 鑳藉惎鍔級锛泇10 鐨?panic 鏉ヨ嚜 **ICC** 鎴栦笁鑰呯粍鍚?2. UFS 璧蜂笉鏉ユ槸鍥犱负**缂哄皯 ICC + DMA-IOMMU 閰嶅**銆傝瘉鎹細鍘熷巶绯荤粺锛圱WRP dmesg 鍙锛夋寜杩欎釜椤哄簭鍔犺浇妯″潡锛?   `icc-bcm-voter.ko 鈫?icc-debug.ko 鈫?icc-rpmh.ko 鈫?iommu-logger.ko 鈫?msm_dma_iommu_mapping.ko 鈫?phy-qcom-ufs*.ko 鈫?ufshcd-*.ko 鈫?ufs_qcom.ko`
3. `logdump` = `/dev/block/sde66`锛?12 MB 鉁?
**涓嬩竴姝ワ紙鎸変紭鍏堢骇锛?*锛?1. 鎶?**ICC 鍗曠嫭**鍔犲洖锛坄INTERCONNECT_QCOM` + `INTERCONNECT_QCOM_WAIPIO`锛屼笉鍔犲叾瀹冿級娴嬩竴杞?鈫?纭 ICC 鏄惁灏辨槸 v10 鐨?panic 婧?2. 鑻?ICC 涓€鍔犲氨 panic锛氭妸 ICC 鍋氭垚**妯″潡**锛?m锛? 鍦?init 閲?`finit_module`锛岄伩寮€鏃╂湡鍒濆鍖栨椂搴?3. 琛?`msm_dma_iommu_mapping`锛圙KI 鐨?dma-iommu mock锛変笌 `iommu-logger`锛孶FS 鎵嶈兘鎷垮埌 DMA 鍩?4. UFS 閫氫簡浠ュ悗 `logdump` 灏辨湁鍐呭锛孧0a 闂幆 鈫?杩?M0b

### 13.23 妯″潡鍖?UFS 涓?瀹樻柟鍐呮牳 + 鎴戜滑鐨?init"涓ゆ灏濊瘯锛?026-11-17 21:05锛?
**v14锛堟ā鍧楀寲瀛樺偍閾撅級**锛歚MODULES=y`锛屾妸 `clk-rpmh / icc-rpmh / rpmh-regulator / icc-bcm-voter / icc-debug / pinctrl-waipio / phy-qcom-ufs* / qcom_iommu_util / spmi-pmic-arb / ufs_qcom / ufshcd-*` 鍏?**23 涓ā鍧?*鎸夊師鍘傞『搴忓啓杩?`modules.load`锛岀敱 init 閫愪釜 `finit_module`銆?
- 缂栬瘧涓澶栬俯鍒帮細`ufs_qcom.ko` 寮曠敤 `ufshcd` 鐨?tracepoint锛屾湭琚鍑?鈫?鍦?`ufshcd.c` 鏈熬琛?`EXPORT_TRACEPOINT_SYMBOL_GPL(ufshcd_command/ufshcd_upiu)` 鍚庨€氳繃锛?*33 涓ā鍧楀叏閮ㄧ紪鍑?*
- 鐪熸満锛?*榛戝睆鍏虫満锛坕nit 璺戜簡锛?*锛屼絾 `logdump` 浠嶄负绌?鈫?UFS 渚濇棫娌¤捣鏉?
**v15锛堝畼鏂瑰唴鏍?+ 鎴戜滑鐨?init 鏀捐繘 vendor ramdisk锛?*锛歚repack_vendor_boot.py --add-init`锛岃嚜妫€ `init=True modules.load=True dtb=True avb=True`銆傜湡鏈猴細**寮€鏈哄け璐?鈫?鑷姩杩?TWRP**锛宍logdump` 浠嶇┖銆?
### 13.24 褰撳墠鐨勬牳蹇冮殰纰嶏細娌℃湁鍐呮牳鏃ュ織閫氶亾

鎵€鏈夊け璐ラ兘鍗″湪鍚屼竴浠朵簨涓?鈥斺€?**鐪嬩笉鍒板唴鏍歌緭鍑?*锛?
| 閫氶亾 | 鐘舵€?|
|---|---|
| 涓插彛 `ttyMSM0` | 鏃犵墿鐞?UART |
| pstore/ramoops | DT 閲屾槸 **鍔ㄦ€佸垎閰嶅湴鍧€**锛圱WRP dmesg: `0x240000@0xa7c5c0000`锛夛紝姣忔鍚姩鍦板潃涓嶅悓锛岃褰曚笉璺ㄩ噸鍚?|
| `logdump` 瑁稿垎鍖?| 闇€瑕?UFS锛岃€?UFS 璧蜂笉鏉?鈫?姝诲惊鐜?|
| 鏄剧ず/fbcon | 鍐呮牳娌℃湁鏄剧ず椹卞姩锛坱echpack/display 鏈紪锛?|

**涓嬩竴姝ュ€欓€夛紙鎸夋€т环姣旀帓搴忥級**锛?
1. **缁?DTB 鐨?ramoops 鑺傜偣鏀规垚鍥哄畾 `reg`**锛堟妸 `alloc-ranges` 鎹㈡垚鍥哄畾鐗╃悊鍦板潃锛夛紝杩欐牱 panic/console 鏃ュ織鑳借法閲嶅惎淇濈暀锛涘啀璁?TWRP 鎴栦笅涓€娆″惎鍔ㄨ鍑烘潵 鈥斺€?杩欐槸**鍞竴鑳芥嬁鍒板唴鏍告棩蹇?*鐨勭‘瀹氬姙娉?2. **缂栨樉绀洪┍鍔?*锛坱echpack/display 鈫?`msm_drm`锛夛紝鐢?fbcon 鎶婃棩蹇楁墦鍒板睆骞曚笂锛岀敤鎴锋媿鐓?3. 鏌ユ竻"瀹樻柟鍐呮牳 + vendor ramdisk 閲岀殑 /init"涓轰綍涓嶇敓鏁堬紙鍙兘 ABL 鍙В鍖?generic ramdisk锛屾垨 vendor ramdisk 鐨?/init 琚?Android init 瑕嗙洊锛?4. UFS 鍗曠嫭娣辨寲锛氬姣斿師鍘?`ufs_qcom.ko` 涓庢垜浠殑鐗堟湰鍦?`ufs_qcom_init` 涓婄殑宸紓锛堟椂閽熻〃銆丳HY 鍒濆鍖栥€乣qcom,ufs-bus-bw` ICC 璺緞锛?
### 13.25 宸茶瘉瀹炰笌宸叉帓闄わ紙渚夸簬閲嶅惎鍚庣户缁級

| 缁撹 | 璇佹嵁 |
|---|---|
| 鎴戜滑鐨?init 鑳藉湪杩欏彴鏈哄櫒涓婁互 PID 1 杩愯 | 澶氭"鍒峰畬鑷姩鍏虫満" |
| 鑷紪鍐呮牳锛堜竴鍔?OSS 鏍?GKI 閰嶇疆锛夎兘鍚姩 | v4/v11/v12/v13/v14 鍏ㄩ儴鍚姩 |
| SMMU 寮€缈昏瘧浼?panic | v10 panic锛寁12 鍔?`iommu.passthrough=1` 鍚庢甯?|
| ICC 鏃犲 | v13 鍚姩姝ｅ父 |
| 妯″潡鍖栧瓨鍌ㄩ摼涓嶇牬鍧忓惎鍔?| v14 榛戝睆鍏虫満 |
| LineageOS 鏍戝唴鏍镐笉閫傞厤鏈満 DTB | v9 panic 閲嶅惎銆乿8 鍗＄涓€灞?|
| `misc` 鍏ㄩ浂鏄甯哥姸鎬?| 绾垮埛鍚?dump 浠嶆槸鍏ㄩ浂 |
| `recovery` 闇€瑕佸畼鏂?`boot_a` | 涓嶅埛瀹樻柟 boot 鏃?TWRP 璧蜂笉鏉?|
| vendor_boot 閲嶆墦鍖呭彲閫愬瓧鑺傝繕鍘?| `--self-test identical=True` |

### 13.26 鏃ュ織閫氶亾鏀诲叧锛氬浐瀹?ramoops + 涓插彛鎺у埗鍙帮紙2026-11-17 21:55锛?
涓轰簡璁╁唴鏍告棭鏈熷穿婧冧篃鑳界暀鐥曪紝鍋氫簡杩欎簺锛?
1. **DTB 鍘熷湴琛ヤ竵**锛坄patch_dtb_inplace.py`锛夛細`ramoops_region` 鐨?`alloc-ranges` 浠?鍏ㄥ湴鍧€绌洪棿"鏀规垚鍥哄畾绐楀彛
   `<0x0a 0x7c5c0000 0x00 0x240000>` 鈥斺€?**16 瀛楄妭绛夐暱鏇挎崲锛孌TB 鎬婚暱搴︿竴瀛楄妭鏈彉**锛孉VB footer 淇濈暀
   - 缁撴灉锛歍WRP dmesg 纭 `ramoops: using 0x240000@0xa7c5c0000`锛堝浐瀹氬湴鍧€鐢熸晥 鉁咃級
   - 涔嬪墠"閲嶅缓 DTB"锛堝昂瀵?+22 瀛楄妭锛変細瀵艰嚧**寮€鏈哄け璐ヨ繘 TWRP**锛岃瘉鏄庡昂瀵告晱鎰熺殑鍧戠湡瀹炲瓨鍦?2. **vendor_boot cmdline 琛ヤ竵**锛坄patch_vb_cmdline.py`锛夛細鍘熷巶 cmdline 閲屽啓鐫€ `msm_geni_serial.con_enabled=0`
   锛?*涓插彛琚樉寮忓叧闂?*锛夛紝鏀规垚 `=1` 骞跺姞 `console=ttyMSM0,115200n8 loglevel=8`
   - 鍏抽敭璁ょ煡锛?*鍐呮牳 cmdline 鏉ヨ嚜 vendor_boot**锛宍boot.img` 鑷繁鐨?cmdline 鏄┖鐨?3. **涓插彛椹卞姩**锛歚SERIAL_MSM_GENI` 渚濊禆 `MSM_GENI_SE`锛堜笉璁剧疆浼氶潤榛樹笉鐢熸晥锛夛紝涓や釜閮藉紑鎴?`=y`锛?   鍙﹀ `drivers/platform/msm` 鐨?`msm-geni-se.c` 闇€瑕?`-I$(src)` 鎵嶈兘鎵惧埌 `qup-common-trace.h`
4. **璁?init 涓诲姩 panic**锛坄/proc/sysrq-trigger` 鍐?`c`锛夆€斺€斿洜涓?console/dmesg 璁板綍**鍙湪 panic/oops 鏃跺畾绋?*锛屽共鍑€鍏虫満涓嶄細鎻愪氦

**缁撴灉锛氫粛鐒舵槸 `/sys/fs/pstore` 涓虹┖銆?* 璇存槑 TWRP 鍚姩鏃朵細鎶?ramoops 鍖哄煙閲嶆柊鍒濆鍖栵紙瑕嗙洊鎴戜滑鍐呮牳鐣欎笅鐨勬暟鎹級锛?鎴栬€呮垜浠殑鍐呮牳鍦?panic 鍓嶆牴鏈病娉ㄥ唽涓婃帶鍒跺彴銆備笅涓€娆¤楠岃瘉鐨勬柟鍚戯細

- 鐢?*鍚屼竴涓唴鏍?*鍚姩涓ゆ锛氱涓€娆?panic锛岀浜屾锛堝悓鍐呮牳锛夎 pstore 鈥斺€?閬垮紑 TWRP 瑕嗙洊
- 鎴栬€呮妸 `/proc/sysrq-trigger` 鎹㈡垚 `panic_on_oops` + 鏁呮剰 oops锛屽苟纭 `dmesg-ramoops-0` 鏄惁鐢熸垚
- 鎴栬€呭共鑴嗘斁寮?pstore锛屾敼涓?*璁?init 鎶?dmesg 鍐欒繘 `logdump`**锛堜竴鏃?UFS 閫氫簡灏辫嚜鐒舵垚绔嬶級















---

## Appendix A - 2026-11-18, further kernel bisection and prior art

### A.1 Built-in vs modular matters, and it is not the clocks

| image | SMMU | result |
|---|---|---|
| v12 | **module** + `iommu.passthrough=1` | boots, `/init` runs |
| v36/v37 | **built-in** + `iommu.passthrough=1` | stuck at the boot logo |

Same driver, same cmdline - only the initcall timing differs. So the hang is an
*early built-in probe*, not the clock values themselves.

Configurations that all hung (stuck at logo or ABL's 2-minute timeout):

* `rpmhpd` as a module (v27) and built in (v28, v29)
* `gcc-waipio` as a module (v26) and built in (v36)
* the OEM's own 18 modules with their vermagic rewritten (v30) - the kernel has
  `CONFIG_MODVERSIONS=y`, so the CRCs are checked anyway; building with
  MODVERSIONS=n avoided that but the modules then hung as well
* LineageOS tree + `gki_defconfig` + `waipio_GKI.config` (v33)
* LineageOS tree + `gki_defconfig` + `waipio_consolidate.config` (v35, 1882 built-ins)

### A.2 `vendor_boot` cannot be modified even with a re-signed footer

The internal AVB hash descriptor covers the vendor ramdisk. Replacing it with a
footer signed by a test key (`avbtool add_hash_footer`, SHA256_RSA2048) is still
rejected: OnePlus ABL verifies against the OEM key. `boot.img` on the other hand
is accepted with `vbmeta` flags=3, which is why a self-built kernel is the only
route.

### A.3 Prior art that changes the plan

* [OpenHarmony rich-device porting guide (Diemit, Xiaomi 6 / SDM835)](https://ost.51cto.com/posts/20849)
  - he did **not** build a kernel from scratch; he used the vendor's own kernel
    source plus the vendor build scripts, then applied OpenHarmony patches.
    His `boot.img` cmdline contains three things worth copying:
    `clk_ignore_unused`, `default_boot_device=<ufshc node>` and
    `ohos.required_mount.<name>=<dev>@<mountpoint>@<fstype>@<options>@<flags>`
    (the last one removes the need for our `/etc/fstab.required`).
    `clk_ignore_unused` did **not** fix our hang (v37), but the other two are
    still useful.
* [OpenHarmony kernel patch set for 5.10](https://gitee.com/openharmony/kernel_linux_patches)
  (`linux-5.10/common_patch/hdf.patch` + per-SoC patches) and the corresponding
  kernel tree `kernel_linux_5.10`, which carries the access_token / binder
  extensions.
* [HarmonyOS 4 GSI for Qualcomm devices](https://www.coolapk.com/feed/48816701)
  exists but is explicitly a toy: brightness, camera, calls and fingerprint are
  broken, and it still needs kernel work ("华为的开源内核魔改严重").

### A.4 Conclusion

The remaining credible path is to reproduce the vendor's *build system* rather
than the config: AOSP kernel prebuilts (clang-r416183b), `build.config.msm.waipio`,
LTO+CFI enabled, the vendor's DTB - verify that this boots, then apply the
OpenHarmony patches (HDF + access_token/binder). Every ad-hoc `make` build we
tried loses to a driver probing too early, and LTO+CFI+the OEM toolchain is the
one variable never matched.
---

## Appendix B - 2026-11-18 evening: OpenHarmony's init really runs on the phone

This session took the port from "partitions exist" to "HarmonyOS init executes on
real hardware".  Everything below is measured on the NE2210, not inferred.

### B.1 The winning boot chain

`boot.img` = **stock OnePlus kernel (gzip, untouched)** + **HarmonyOS ramdisk**
(the 6.5 MB tree from the Pura-70 ramdisk) + **our own `/init`**.

`vendor_boot_a` stays stock: the stock vendor ramdisk is merged by the kernel and
supplies `/lib/modules` (144 modules) to whatever ramdisk boots.  `boot.img` is
the only modifiable piece - ABL rejects any modified `vendor_boot` even when its
AVB footer is re-signed with a test key.

Boot sequence that works:

1. `/init` mounts proc/sys/dev(tmpfs)/tmp
2. loads `/lib/modules/modules.load` in **multi-pass** fashion (dependencies
   resolve over repeated passes; single pass leaves 65 of 144 unloaded)
3. waits for block devices (UFS probes asynchronously; 131 appear)
4. creates `/dev/mapper/control` and `/dev/device-mapper` (both `c 10:236`) -
   **without these every dm ioctl fails silently and zero partitions get mapped**
5. parses the super LP metadata and creates dm-linear mappings
6. creates `/dev/block/dm-N` nodes **after** mapping (`uevent` scan)
7. mounts `dm-0` (the HarmonyOS `system` partition, erofs) at `/usr`
8. makes `/usr/system` reachable as `/system` (early symlink works; bind mount
   returns E2BIG on this kernel)
9. mounts tmpfs on `/data`, `/dev/pts`, `/dev/shm`
10. `exec /system/bin/init`

### B.2 Layout facts (these cost hours)

* the HarmonyOS `system` image is mounted at **`/usr`**;
* its top level is a symlink farm (`bin -> /system/bin`, `init -> /system/bin/init`)
  and the real tree lives in its **`system/` sub-directory**;
* therefore `/system` must resolve to **`/usr/system`**;
* `/system/bin/init` is 456 KB, dynamically linked against
  `/system/lib/ld-musl-aarch64.so.1`.

### B.3 Proof that HarmonyOS init executes

Forked child (PID != 1) running `/system/bin/init`:

```
child pid=245
child alive at 5s
child exited (status 0x0) at 10s
```

As PID 1 it starts and keeps running, but the phone resets after ~80 s.  Feeding
the Qualcomm watchdog (persistent fd, pings every 5 s) does **not** change that,
so the reset is a **kernel panic**, not the watchdog: OpenHarmony's init reaches
code that needs OpenHarmony kernel features (the tokenid / access_token driver is
the usual first one) and the stock kernel has none of them.

### B.4 Debug channel without UART, display or adb

`logdump` partition, raw, no filesystem needed:

```c
/* open /dev/block/<logdump>, pwrite 256-byte slots at 4096 + n*256 */
```

Read it back from TWRP:

```sh
dd if=/dev/block/by-name/logdump bs=4096 skip=1 count=24 | strings
```

This turned every ambiguous "stuck at logo" into an exact answer and is the
single most useful tool of the session.

### B.5 Two bugs worth remembering

* dm minors start at **0**, so `if (!system_minor)` treats a valid dm-0 as
  "not found" - keep a separate `have_system` flag;
* the device nodes for dm devices must be created **after** the mappings exist,
  otherwise `mount("/dev/block/dm-0")` fails with ENOENT.

### B.6 What is left

The userspace side of M0b is done; the remaining blocker is the kernel.  The
stock kernel cannot be patched and every self-built kernel either lacks UFS or
freezes on early probe, so OpenHarmony's kernel features (access_token/tokenid,
binder extensions, hmdfs) cannot be added yet.  The next milestone is a
self-built kernel that boots *and* has UFS, i.e. reproducing the vendor build
system (clang-r416183b, `build.config.msm.waipio`, LTO+CFI) - see Appendix A.4.
### B.7 Control experiment: the reset is caused by OpenHarmony's init

Two boots of the same image, differing only in whether OpenHarmony's init is
executed:

| variant | behaviour |
|---|---|
| full chain, then **park forever** instead of exec | stays at the boot logo **indefinitely** |
| full chain, then `exec /system/bin/init` | resets after **~80 s** |

So the ~80 s reset is **not** a bootloader or watchdog timeout: it is a kernel
panic caused by OpenHarmony's init.  (`qcom_wdt_core`, `qcom_enhance_watchdog`,
`qcom_soc_wdt` and `init_watchdog` all load fine; feeding a watchdog device is
irrelevant because this kernel keeps the watchdog entirely in a kernel thread and
exposes no `/dev/watchdog` at all.)

That narrows the remaining work to exactly one thing: the stock kernel lacks the
OpenHarmony kernel interfaces (tokenid / access_token, binder extensions, ...),
and a stock kernel cannot be patched - `vendor_boot` is signature-locked and only
the `boot.img` ramdisk is replaceable.

Capturing the panic trace was not possible from userspace: `panic()` stops every
other CPU, so a forked logger never runs again, and neither `logfs` (8 MB, empty)
nor `logdump` (512 MB, no kernel data) receives the dump on this device.

### B.8 Next steps, in order

1. Trace OpenHarmony's init syscalls with `ptrace` from our own init (the parent
   stays alive, logs the last syscalls into `logdump`), with the child made PID 1
   of a new PID namespace so it still performs its boot sequence.  That names the
   exact missing interface.
2. Build the missing interfaces into a kernel we can boot: reproduce the vendor
   build system (clang-r416183b, `build.config.msm.waipio`, LTO+CFI) so UFS comes
   up in a self-built kernel, then apply the OpenHarmony patches.
---

## Appendix C - 2026-11-18 late: we can now build a kernel that boots

### C.1 The vendor build recipe, recovered from a prebuilt kernel

A working third-party kernel for this exact device exists
([WildKernels/OnePlus_KernelSU_SUSFS](https://github.com/WildKernels/OnePlus_KernelSU_SUSFS)).
Its `Image` reports:

```
Linux version 5.10.236-android12-OP-WILD (OnePlus@ubuntu-build)
(Android (7416131, based on r416183c) clang version 12.0.6, LLD 12.0.6)
```

`configs/a16/OP10pro.json` + `manifests/a16/oneplus_10_pro_w.xml` in that repo give
the complete recipe:

| piece | source |
|---|---|
| kernel | `OnePlusOSS/android_kernel_common_oneplus_sm8450` @ `oneplus/sm8450_b_16.0_oneplus_10_pro` |
| modules + devicetree | `OnePlusOSS/android_kernel_modules_and_devicetree_oneplus_sm8450` @ same branch |
| toolchain | `git.codelinaro.org/clo/la/kernelplatform/prebuilts-master/clang/host/linux-x86` (`clang-r416183c`) |
| build tools | `git.codelinaro.org/clo/la/kernel/prebuilts/build-tools` |

Build environment that worked:

```sh
export LLVM=1 LLVM_IAS=1 ARCH=arm64 SUBARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
export LD=ld.lld HOSTLD=ld.lld AR=llvm-ar NM=llvm-nm
export OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump STRIP=llvm-strip
# ld-wrapper: ld.lld "$@" --thinlto-jobs=$(nproc)
make LD=$PWD/ld-wrapper HOSTLD=$PWD/ld-wrapper O=out gki_defconfig
make LD=$PWD/ld-wrapper HOSTLD=$PWD/ld-wrapper O=out olddefconfig
make LD=$PWD/ld-wrapper HOSTLD=$PWD/ld-wrapper -j$(nproc) O=out Image modules
```

Two pitfalls worth remembering:

* `LD` must be an **absolute** path (`./ld-wrapper` is resolved inside `out/` and
  the kconfig step then fails with "linker './ld-wrapper' not found");
* the build scripts call `python`, not `python3` - create the symlink.

With **clang-r416183c** `CONFIG_LTO_CLANG_THIN=y` is finally accepted; with the
distribution clang 18 the same switch is silently dropped to `CONFIG_LTO_NONE`.

Tree layout matters: the kernel must sit in `kernel_platform/common` with
`vendor/` at the checkout root, because the tree is full of relative symlinks
(`common/kernel/oplus_cpu -> ../../../vendor/oplus/kernel/cpu`).

### C.2 Result: a self-built kernel boots on the phone

`Image` (39.6 MB, LTO thin, 68 modules) + the minimal v6 logger ramdisk:

```
~12 s -> screen off, i.e. /init ran and powered the device off
```

So the self-built kernel boots.  This is the piece that blocked everything: from
now on the kernel can be patched freely, which is what OpenHarmony's kernel
features (tokenid/access_token, binder extensions) require.

### C.3 Still missing: the vendor clock/PHY/interconnect modules

`CONFIG_SCSI_UFS_QCOM=m` builds and its vermagic matches our kernel, but the UFS
probe also needs `gcc-waipio`, `phy-qcom-ufs-qmp-v4-waipio`, `icc-rpmh` and
`rpmh-regulator`, which are **not** built by `gki_defconfig` + the waipio fragment
(`MSM_GCC_WAIPIO`, `PHY_QCOM_UFS`, `PINCTRL_WAIPIO`, `INTERCONNECT_QCOM_WAIPIO`
all end up unset).  They come from the vendor tree and are built by the platform
module build, which needs `kernel_platform/msm-kernel` - the symlink
`build.config.msm.waipio -> ../msm-kernel/build.config.msm.waipio` dangles in our
checkout, and that repository is not in the manifest.

Note for the next attempt: the WildKernels kernel is **GKI only** (its embedded
config has `# CONFIG_SCSI_UFS_QCOM is not set` and no vendor drivers at all); it
works on devices because it keeps `CONFIG_LOCALVERSION` compatible with the stock
modules.  Two viable routes therefore exist:

1. build the vendor modules ourselves (`make M=vendor/qcom/opensource/...` against
   our kernel) and ship them in `/ohos-modules` inside our ramdisk, or
2. fetch `msm-kernel` so the platform module build works.
### C.4 Why "impersonating" the stock kernel does not work

Attempt: build a GKI-shaped kernel whose `UTS_RELEASE` is forced to the stock
string (`scripts/setlocalversion` made to print `-gki-g0b92723d0f5a`), keep
`CONFIG_MODVERSIONS=y` so the vermagic matches byte for byte, and patch
`check_version()` in `kernel/module.c` to `return 0` so the symbol CRCs are not
compared.  Then the stock vendor modules should load.

Measured result:

* the self-built kernel boots (verified with the minimal logger ramdisk: powers
  off after ~14 s);
* but booting it with the OpenHarmony ramdisk - which loads the stock modules -
  resets after ~12 s and our init never reaches its first log line.

The reason is that skipping the CRC check only bypasses the *detection* of an ABI
mismatch: the stock modules are compiled against the stock configuration, and our
kernel's configuration differs (struct layouts, lockdep, etc.).  Loading them
corrupts the kernel instead of failing cleanly.

Conclusion: the vendor drivers must be built **from source against our own
kernel** - `gcc-waipio`, `phy-qcom-ufs-qmp-v4-waipio`, `icc-rpmh`,
`rpmh-regulator`, `pinctrl-waipio`.  They live in Qualcomm's `msm-5.10` tree
(reachable at `git.codelinaro.org/clo/la/kernel/msm-5.10`, tags
`KERNEL.PLATFORM.1.0.r1-*` / `1.1.r1-*`), which is the missing
`kernel_platform/msm-kernel` that `build.config.msm.waipio` points at.