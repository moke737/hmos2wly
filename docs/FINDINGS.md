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
---

## Appendix D - 2026-11-18 late: the full vendor platform build works

The missing piece from Appendix C was Qualcomm's own kernel tree.  It is public:

```
git clone --depth 1 -b KERNEL.PLATFORM.1.0.c25 \
  https://git.codelinaro.org/clo/la/kernel/msm-5.10  kernel_platform/msm-kernel
```

With it in place, OnePlus's own platform build runs:

```sh
cd kernel_platform
BUILD_CONFIG=msm-kernel/build.config.msm.waipio ./build/build.sh
```

### D.1 What had to be fixed to make the platform build run on a plain Ubuntu

| problem | fix |
|---|---|
| `_setup_env.sh` replaces PATH with only the prebuilt host tools (`nproc`, `sed`, `readlink` disappear) | append `:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin` to that PATH assignment |
| host compiles cannot find `sys/types.h` | point `build/build-tools/sysroot` at `/` |
| `bison`/`flex` wrappers point at a prebuilt that is not fetched | symlink the wrappers to `/usr/bin/bison`, `/usr/bin/flex`, `/usr/bin/m4` |
| `external/dtc` missing | shim Makefile that installs the **system** `dtc` (the prebuilt dtc needs `libfdt.so`) |
| ABI sanity check aborts on `abi_gki_aarch64.xml` mismatch | early `return 0` in `sanity_check_abi()` (`msm-kernel/build.config.msm.gki`) |
| `clang-r416183b` expected | symlink it to the fetched `clang-r416183c` |

### D.2 Result

```
out/msm-waipio-waipio-consolidate/dist/Image      56.8 MB   (full vendor kernel)
out/.../staging/lib/modules/5.10.233-qki-consolidate-dirty/   668 modules
```

Every driver the port needs is now built from source for our own kernel:

```
gcc-waipio.ko  phy-qcom-ufs.ko  phy-qcom-ufs-qmp-v4-waipio.ko  ufs_qcom.ko
icc-rpmh.ko  icc-bcm-voter.ko  rpmh-regulator.ko  rpmhpd.ko  clk-rpmh.ko
clk-qcom.ko  spmi-pmic-arb.ko  pinctrl-waipio.ko  arm_smmu.ko  cmd-db.ko  smem.ko
```

(the build only fails at the very last dtbo packaging step, `mkdtboimg.py` with an
unmatched glob - irrelevant for the kernel and modules)

### D.3 Current blocker

Booting `platform Image + OpenHarmony ramdisk + 90 of those modules` resets after
~13 s with no data in `logdump` - and `logdump` is exactly what cannot be written
when UFS is missing, so the failure is invisible again.  Two facts are established:

* a **GKI build** of the same tree (no vendor drivers) boots fine (minimal ramdisk
  powers off after ~14 s), so the toolchain and packaging work;
* the platform kernel with our init + modules **panics** - either a module probe
  crashes before its suppliers are up (loading is already dependency-ordered from
  `modules.dep`) or PID 1 dies, which panics the kernel by definition.

The next diagnostic is the platform Image with the **minimal v6 logger ramdisk**:
if that powers off normally the kernel is fine and the fault is in the init/module
path; if it resets too, the platform kernel itself is the problem.

### D.4 Useful invariant discovered

`logdump` can only be written from a kernel that already has UFS, so it is useless
for diagnosing storage bring-up.  Use the power-off delay encoding instead
(`sleep(10 + value); reboot(RB_POWER_OFF)`), which needs nothing but a working
kernel.
---

## Appendix E - 2026-11-19 early morning: the observability wall

### E.1 What was established tonight

* the full OnePlus/Qualcomm platform build works (Appendix D): `Image` + 668 modules;
* that `Image` **boots** - with the minimal logger ramdisk it powers off after
  12-15 s, exactly like the GKI build;
* module loading from `/ohos-modules` works in principle: loading only
  `smem.ko` produced a clean power-off (delay encoding confirmed it);
* **`qcom_glink.ko` panics the kernel** - bisected over the ordered list:
  `smem` OK, `smem+qcom_smd+minidump+qcom_ipc_logging` OK, adding
  `qcom_glink` resets.  GLINK's probe talks to a remoteproc that is not up in an
  initramfs-only boot.
* **but the rest of the pipeline still panics even when the module set is
  emptied** - with module loading stubbed out the kernel still resets, and since
  PID 1 dying panics the kernel by definition, every fault in `/init` is fatal and
  invisible.

### E.2 The observability wall

Diagnosing that needs a log channel, and every candidate is blocked:

| channel | why it fails |
|---|---|
| `logdump` partition | needs UFS, which is exactly what we are debugging |
| `pstore`/ramoops | the DTB node uses dynamic `alloc-ranges`, and reading it requires a later boot that can write somewhere anyway |
| USB serial console | the kernel has `CONFIG_USB_CONFIGFS(_SERIAL)=y` and the Image contains `ttyGS`, but `dwc3-msm` needs the GLINK/PMIC role-switch path - and `qcom_glink` is the module that panics |
| visible LEDs / backlight | both are provided by modules that are not up that early |

### E.3 Two ways out

1. **A supervisor `/init`** (implemented, `m0b/supervisor.c`): PID 1 only mounts
   proc/sys/dev, forks `/realinit`, polls a `/stage` file and, if the child dies,
   encodes the last stage in the power-off delay (`10 + stage`).  A crash in the
   worker no longer panics the kernel, so the stage becomes readable.  The image
   was built but not yet tested when the session ended.
2. **Make GLINK survive**: the panic is in its probe; loading `qcom_glink` after
   the remoteproc/aoss pieces (or patching the driver to defer when the SMEM host
   is absent) would restore the USB gadget and with it a real console.

### E.4 Lesson worth keeping

For bring-up work the order of value is: **serial console > USB gadget console >
raw partition logging > timing codes**.  Building the console first would have
saved most of this session.
### E.5 Bisect results (module set)

Loading the vendor modules one by one into the platform kernel, using the power-off
delay to report (`10 + n_ok`), produced a clear chain:

| modules loaded | result |
|---|---|
| `smem` alone | power off, clean |
| `smem, qcom_smd, minidump, qcom_ipc_logging` | power off at ~14 s, clean |
| + `iommu-logger` | **kernel panic** |
| + `qcom_glink` (earlier list) | **kernel panic** |

So `iommu-logger.ko` and `qcom_glink.ko` both panic this kernel during probe.
Neither is needed for UFS, so both are excluded (`m0b/rd_order_v2.txt`).

Important: these panics are **kernel-level**, which is why the supervisor design
(`m0b/supervisor.c`, PID 1 supervises a child) does not help - a panic takes the
whole machine down regardless of which process triggered it.  The supervisor only
protects against userspace faults.

### E.6 Built-in storage stack: the export problem

The other way to avoid the fragile module loader is to build the storage stack
into the kernel.  Flipping the vendor fragments (`waipio_GKI.config`,
`waipio_consolidate.config`) from `=m` to `=y` for
`QCOM_RPMH, QCOM_RPMHPD, QCOM_CLK_RPMH, REGULATOR_RPMH, MSM_GCC_WAIPIO,
SCSI_UFS_QCOM, PHY_QCOM_UFS(_V4), PINCTRL_*, INTERCONNECT_QCOM(_WAIPIO),
ARM_SMMU, QCOM_SCM, COMMON_CLK_QCOM` runs into two hard requirements:

1. a tristate can only be `=y` when **all its dependencies are `=y`** - the build
   reports `Defconfig fragment did not apply as expected: X y -> m` and stops, so
   the whole parent chain has to be flipped too (`QCOM_GLINK` cannot be built in
   at all, its dependency stays modular);
2. once a driver is built in, its symbols are **no longer exported to modules**,
   so every remaining module that uses them fails modpost:

```
ERROR: modpost: "msm_pinctrl_probe" [drivers/pinctrl/qcom/pinctrl-cape.ko] undefined!
ERROR: modpost: "qcom_scm_io_writel" [drivers/irqchip/qcom-pdc.ko] undefined!
```

The fix is either `EXPORT_SYMBOL()` on those helpers, or keeping exactly the
subsystems modular that other modules link against.  The narrowest useful set to
build in is probably only the UFS chain
(`QCOM_RPMH, QCOM_RPMHPD, QCOM_CLK_RPMH, REGULATOR_RPMH, MSM_GCC_WAIPIO,
SCSI_UFS_QCOM, PHY_QCOM_UFS, PHY_QCOM_UFS_V4`) while leaving `PINCTRL_MSM`,
`QCOM_SCM`, `ARM_SMMU` and `INTERCONNECT_QCOM*` as modules.
### E.7 Path A is architecturally impossible: this is a GKI 2.0 mixed build

Attempting to build the UFS chain into the kernel never converges, and the reason
turned out to be structural rather than a config mistake.  In the build output:

```
out/msm-waipio-waipio-consolidate/
    gki_kernel/common/Module.symvers     <- the GKI kernel is built here
    msm-kernel/drivers/...               <- no vmlinux, no .o files
```

`msm-kernel` is compiled as an **external module set against the prebuilt GKI
kernel**.  Every vendor driver - `ufs_qcom`, `gcc-waipio`, `rpmhpd`,
`phy-qcom-ufs`, `icc-rpmh`, `pinctrl-waipio` - can only ever be a module in this
platform.  That is also why each `=y` in a fragment is silently forced back:

```
ERROR! Defconfig fragment did not apply as expected
  QCOM_RPMH y -> m
  SCSI_UFS_QCOM y -> m
```

`IOMMU_IO_PGTABLE_FAST` and `QCOM_GDSC` are `bool`, so `=m` is rejected for them
(they must be `=y`), and a tristate can only be `=y` when its whole dependency
chain is `=y` - which GKI forbids for vendor code.

**Conclusion: path A is dead.  Loading the vendor modules at boot is the only
route on this platform**, so the work has to go into making module loading
survive: the panics are in individual drivers' probe paths, and the two confirmed
ones (`iommu-logger`, `qcom_glink`) are both dispensable for storage.

### E.8 Practical bisect recipe for the next session

1. take `m0b/rd_order_v2.txt` (83 modules, `iommu-logger` and the glink family
   already removed);
2. build prefix images with `m0b/build_v34.sh <N>` (the init reports
   `10 + n_ok` through the power-off delay);
3. a reset means the culprit is inside the first N; a clean power-off means it is
   later - four or five boots identify it;
4. exclude it and repeat.
---

## Appendix F - 2026-11-19: how the known-good ports actually do it

Studied the Xiaomi 6 / SDM835 port again
([Diemit's porting guide](https://ost.51cto.com/posts/20849),
[packaging and debugging article](https://m.elecfans.com/article/2012725.html)).
Three details matter for us:

### F.1 They build every driver INTO the kernel

> "我们的内核因为把模块编译进内核的缘故，容量会增大" - boot.img grows from 8 MB to
> 16 MB because all modules are compiled in.

That is only possible because the Xiaomi 6 kernel is a classic non-GKI tree.  Our
platform is GKI 2.0 (`msm-kernel` is an external module set), which is exactly why
the built-in attempt failed (Appendix E).

**But `msm-kernel` itself is a complete kernel tree**, and it can be configured
standalone:

```sh
cd kernel_platform/msm-kernel
make O=out-alone ARCH=arm64 vendor/waipio-consolidate_defconfig
# force CONFIG_SCSI_UFS_QCOM / MSM_GCC_WAIPIO / QCOM_RPMHPD / ARM_SMMU ... = y
make O=out-alone ARCH=arm64 Image
```

Standalone, `=y` sticks - a kernel with UFS built in is genuinely possible, which
would remove the fragile module loader from the boot path entirely.

### F.2 The cmdline carries the mounts for HarmonyOS

```
ohos.required_mount.system=/dev/block/.../by-name/system@/usr@ext4@ro,barrier=1@wait,required
ohos.required_mount.vendor=/dev/block/.../by-name/cust@/vendor@ext4@...
```

HarmonyOS init reads these and mounts by itself - no `/etc/fstab.required` needed.
Since `boot.img` is the one image we can modify, this is available to us and is
simpler than the fstab route we currently use.

### F.3 They use a USB serial console as the debug channel

`console=ttyGS0,115200` in the cmdline, then "如果内核驱动正常的话手机使用 usb 连接
电脑会自动识别为 com 设备" - the phone shows up as a COM port.  We have all the
gadget support built in (`CONFIG_USB_CONFIGFS_SERIAL=y`, `ttyGS` in the image);
what was missing is `dwc3-msm`, whose probe needs the GLINK/PMIC role switch.

### F.4 Where the standalone build stands

Without LTO (the thin-LTO link needs more than this machine's 9.7 GB of RAM and
thrashes swap) the standalone kernel compiles and links in ~4 minutes per
iteration.  Progressively fixing undefined symbols found at link time:

| symbol | fix |
|---|---|
| `gdsc_debug_print_regs` | `QCOM_GDSC_REGULATOR=y` |
| `ufshcd_qti_hba_init_crypto_capabilities`, `crypto_qti_disable` | `SCSI_UFS_CRYPTO_QTI=y`, `QTI_CRYPTO_COMMON=y`, then `QTI_HW_KEY_MANAGER=y` (its `crypto_qti_*_platform` live in `crypto-qti-hwkm.c`; `QTI_CRYPTO_TZ` is the alternative implementation and must stay off or the symbols collide) |
| `devm_regulator_debug_register`, `regulator_proxy_consumer_sync_state` | `REGULATOR_DEBUG_CONTROL=y`, `REGULATOR_PROXY_CONSUMER=y` |
| `iommu_logger_register` | `QTI_IOMMU_SUPPORT=y` |
| `hyp_assign_phys` | `QCOM_SECURE_BUFFER=y` |
| `ipc_log_string`, `ipc_log_context_create` | `CONFIG_IPC_LOGGING=y` (implementation is `kernel/trace/ipc_logging.c` in this tree, not `drivers/soc/qcom/`) |
| `tme_hwkm_master_broadcast_transportkey` | comes from `tmecom`, which needs the QTI QMP mailbox driver that this tree lacks - stub it, or leave `MSM_TMECOM_QMP` off |
| `qmp_get`, `qmp_send` | still unresolved; another built-in driver uses QMP and the provider is not in this tree |

So the standalone route is a finite symbol-chasing exercise (~4 minutes per round)
and was left at the QMP symbols.  It is the most promising way to a kernel with UFS
built in, and therefore to a boot in which every driver comes up in kernel order -
the way the Xiaomi 6 port works.
### F.5 The standalone kernel builds, but the full vendor config does not boot

`msm-kernel` configured standalone and built with the vendor's
`waipio-consolidate_defconfig` links successfully (~4 minutes, no LTO):

```
Image 36 MB
CONFIG_SCSI_UFS_QCOM=y   CONFIG_MSM_GCC_WAIPIO=y   CONFIG_QCOM_RPMHPD=y
vermagic 5.10.233-qki-consolidate-dirty
```

But it resets after ~12 s **even with the minimal v6 logger ramdisk**, i.e. the
kernel dies during its own init - before userspace.  The consolidate config builds
hundreds of vendor drivers in, and at least one of them faults during probe.  This
is the same wall the all-built-in GKI attempt hit at the very beginning.

**The fix is to reduce the config, not to add more.**  The plan for the next
session:

1. take the **GKI config that is known to boot** (`common/arch/arm64/configs/gki_defconfig`)
   as the base for the standalone `msm-kernel` build;
2. enable **only** the UFS chain on top of it:
   `QCOM_SCM, QCOM_SMEM, QCOM_COMMAND_DB, QCOM_RPMH, QCOM_RPMHPD, QCOM_CLK_RPMH,
   REGULATOR_RPMH, COMMON_CLK_QCOM, MSM_GCC_WAIPIO, SCSI_UFS_QCOM, SCSI_UFSHCD,
   PHY_QCOM_UFS, PHY_QCOM_UFS_V4` (+ the helper configs found in F.4:
   `QTI_IOMMU_SUPPORT, QCOM_SECURE_BUFFER, REGULATOR_DEBUG_CONTROL,
   REGULATOR_PROXY_CONSUMER, QCOM_GDSC_REGULATOR, SCSI_UFS_CRYPTO_QTI,
   QTI_CRYPTO_COMMON, QTI_HW_KEY_MANAGER, IPC_LOGGING, QCOM_MINIDUMP,
   QCOM_AOSS_QMP, MSM_TMECOM_QMP, IOMMU_IO_PGTABLE_FAST, QCOM_GDSC`);
3. build, flash with the minimal ramdisk, and only then add the OpenHarmony
   ramdisk.

That gives a kernel whose init path is as close as possible to the one that already
boots, plus exactly the storage stack.
### F.6 Empirical rule: no vendor driver may be built in

Three kernels were produced tonight and tested on the device:

| kernel | vendor drivers | boots? |
|---|---|---|
| GKI only (platform build, `common` config) | none | **yes** |
| GKI base + UFS chain built in (`out-gki`, 31 MB) | 30 symbols | **no** |
| full vendor consolidate build (`out-alone`, 36 MB) | hundreds | **no** |
| WildKernels prebuilt (third-party, 44 MB) | none | **yes** |

Every configuration that contains vendor driver code fails to reach userspace, and
that includes a build with only ~30 vendor symbols.  The third-party kernel that
does work on this device is also pure GKI, and it relies on the stock modules.

So the direction is settled: **the kernel that boots is the GKI one, and the vendor
drivers have to arrive as the stock modules** - which is exactly the configuration
where we already measured 131 block devices on the phone.  The only remaining
problem on that path is that *our* loader trips over two of the 144 modules
(`iommu-logger`, `qcom_glink`, both confirmed by bisect); the rest never got tested
because the bisect stopped there.

### F.7 Plan for the next session (shortest path to a working boot)

1. flash the platform-built GKI kernel (the one that boots) with the OpenHarmony
   ramdisk;
2. use `m0b/rd_order_v2.txt` (83 modules, the two known crashers removed) and
   bisect with `m0b/build_v34.sh <N>` / the power-off delay - 4 or 5 boots;
3. with the crashy modules excluded, UFS comes up, the partitions map and
   HarmonyOS init is reached - the state we already proved is reachable, just with
   a stable module set this time;
4. only then think about kernel patches (OpenHarmony's tokenid/binder), which will
   need a custom kernel anyway - but by then the userspace side will be complete.
---

## Appendix G - 2026-11-19: the OEM kernel config, CFI, and the structural wall

### G.1 The OEM kernel configuration was recovered

The stock `boot.img` kernel embeds its config (`CONFIG_IKCONFIG=y`): after gunzipping the
39,720,164-byte kernel, `IKCFG_ST` sits at 21,185,824 and the payload is a 39,854-byte
gzip block.  Decompressed it is the full **6789-line configuration** of the running
kernel (`m0b/stock_config.txt`).  Two facts matter:

```
# CONFIG_SCSI_UFS_QCOM is not set
# CONFIG_ARM_SMMU is not set
# CONFIG_QCOM_RPMH is not set
CONFIG_LOCALVERSION=""
CONFIG_MODVERSIONS=y
CONFIG_LTO_CLANG_THIN=y
CONFIG_CFI_CLANG=y
CONFIG_SHADOW_CALL_STACK=y
```

* the **boot kernel contains no vendor driver at all** - UFS, clocks, RPMh, SMMU all
  arrive as the 144 stock modules from `vendor_boot`;
* the kernel is built with **ThinLTO + CFI + shadow call stack**.

### G.2 CFI was the (first) reason stock modules misbehaved

Every stock module carries `.cfi_jt` symbols, i.e. it was compiled with
`CONFIG_CFI_CLANG=y`.  Our earlier builds removed LTO and CFI (to keep the link inside
this machine's memory) - a CFI-built module loaded into a non-CFI kernel calls through
jump tables the kernel does not provide.

Rebuilding `common` with the OEM config **including** `LTO_CLANG_THIN`, `CFI_CLANG` and
`SHADOW_CALL_STACK` works once WSL gets enough memory (9.7 GB thrashed; 12 GB + 32 GB
swap finished in 458 s), and the result is **byte-for-byte the same size as the OEM
kernel** (39,720,164 bytes), with identical vermagic.

But: the stock modules **still hang** that kernel.  The OEM kernel reports
`5.10.236-o-g5012cb8f472f` while the modules report `5.10.236-gki-g0b92723d0f5a`, and
`5012cb8f472f` does not exist in any branch of
`OnePlusOSS/android_kernel_common_oneplus_sm8450` (four branches were checked), so their
exact source state is not public.

### G.3 The one configuration that works, and where it stops

The only combination that reaches userspace is **stock kernel + our ramdisk + the stock
modules**:

```
init version 15, block devices=131
dm system -> dm-0 ... dm patch -> dm-5
mounting /dev/block/dm-0 at /usr
mount ok, exec OH init as PID 1          <- HarmonyOS init runs
... 82 s later the screen goes black, 17 s after that the phone resets
```

Bisecting the module list showed `phoenix_reboot_speed.ko` (the first entry) **hangs**
a non-Android boot; excluding it lets the pipeline reach HarmonyOS init.  The stock
kernel itself loads it fine, so this too is a difference between our kernels and theirs.

### G.4 The 82 s failure is a kernel fault, not a userspace reboot

Three experiments bound it:

| experiment | result |
|---|---|
| seccomp filter returning EPERM for `reboot()` before `exec` | the 80 s reboot becomes a **black screen** - init's `reboot(panic)` is the thing that was rebooting |
| also disabling sysrq and binding `/proc/sysrq-trigger` to `/dev/null` | unchanged: black screen at ~82 s, reset ~17 s later |
| forked watchdog feeder (survives `exec`, keeps writing to `/dev/watchdog*`) | **no effect** |

So after the reboot request is refused, something inside the kernel faults at ~82 s;
with `panic=0` the kernel hangs and the **hardware** watchdog resets the phone ~17 s
later.  Nothing in userspace can prevent that.

### G.5 Why the panic cannot be read (four channels, all closed)

| channel | why it fails |
|---|---|
| `logdump` (512 MB) | **persists across boots** (verified with a marker) and holds the v15-v27 era log, but none of the newer inits ever wrote to it, even after the device lookup was reimplemented to scan `/sys/class/block/*/uevent` itself |
| pstore/ramoops | the DT *does* have `ramoops_region` and the config has `PSTORE_RAM/CONSOLE` plus `pstore.compress=none` in the cmdline, but `/sys/fs/pstore` is empty in TWRP **and** after a deliberate sysrq crash |
| USB serial console | the kernel has `CONFIG_USB_CONFIGFS_SERIAL` and `ttyGS`, but the gadget needs `dwc3-msm`, whose probe needs GLINK/PMIC - and loading the stock modules into our kernel hangs |
| partition-based dumpers | every dumper needs UFS, i.e. the modules that only load on the stock kernel |

### G.6 What the sagit port actually does (juezhi6668/hmos2sagit, msm8998)

Pulled both repositories; the port is a **9-patch kernel series** plus file-level patches:

| patch | content |
|---|---|
| 0001 | binder access-token and feature-set ioctls |
| 0002 | hmdfs/sharefs emulation at mount time |
| 0003 | `hmos.noreboot=1` (refuses `reboot(panic)` and the `c`/`b`/`o` sysrq keys) and `hmos.permissive=1` |
| 0004 | config: `ACCESS_TOKENID`, `BINDER_SENDER_INFO`, `OHOS_FS_EMU`, `DEVTMPFS`, system as erofs |
| 0005 | **`/dev/access_token_id` driver** (misc device, per-task token) |
| 0006 | keep the legacy mdss panel helpers for fbdev |
| 0008 | carry the access token across `fork()` |
| 0009 | icnss BDF download + `CAL_REPORT` for Wi-Fi |

Plus, from their docs: the cmdline carries `ohos.required_mount.system=...@/usr@erofs@...`
so HarmonyOS mounts its own filesystems; `/data` must be a **dm device**, not the raw
partition; HOS creates **no** `/dev/block/by-name/`; and the display is a **fbdev
composer** because no DRM path was usable.

`0003` is what we reproduced in userspace with seccomp (G.4) - it works, and it proved
that the 82 s event is the reboot request.

### G.7 Honest state

* HarmonyOS init **executes on the device** and runs for ~82 s before the kernel faults;
* the vendor drivers can only be loaded by the stock kernel, and the HarmonyOS kernel
  interfaces (binder token, access_token_id) can only live in a kernel we build;
* those two statements cannot both be satisfied with the sources available publicly.

The remaining ways forward, in order of expected value:

1. **UART console** - the only channel that needs nothing from the kernel's own drivers.
   Without it every failure stays invisible, which is exactly what has been limiting
   every session so far.
2. Find the OEM kernel's exact source state (`-o-g5012cb8f472f`) - possibly a newer
   OnePlus release than the branches published on GitHub.
3. Build the three HarmonyOS patches that *can* be modules against the stock kernel's
   own symbol versions (binder cannot: it is built into the GKI kernel).

### G.8 Session artefacts worth keeping

```
m0b/stock_config.txt              the OEM kernel configuration (6789 lines)
m0b/stock_modules/                the extracted vendor_boot ramdisk (461 modules, modules.load/dep/blocklist)
m0b/preinit_v50.c, v51.c          aliveness probes with a self-contained logdump lookup
m0b/boot_cfi_test.img             CFI+LTO kernel, size-identical to the OEM one
kp2/kernel_platform/common/out-cfi/Image   the CFI/LTO build
```
---

## Appendix H - 2026-11-19 late: every kernel we build is unusable, and why

### H.1 The decisive control experiment

A trivial init (mount proc/sys/dev, `sleep(25)`, `reboot(RB_POWER_OFF)`) was flashed with
each of our kernels.  Every one of them resets at **13 s of black screen + 17 s**:

| kernel | ramdisk / init | result |
|---|---|---|
| `out-cfi` (OEM config, CFI, thin LTO) | trivial idle init | 13 s + 17 s reset |
| `out-gki2` (gki_defconfig, CFI, thin LTO) | trivial idle init | 13 s + 17 s reset |
| `kp-out` (the one that "booted") | lean 392 KB ramdisk, full init | 13 s + 17 s reset |
| `kp-out` | lean ramdisk, idle init | 13 s + 17 s reset |
| `kp-out` | lean ramdisk, watchdog-only init | 13 s + 17 s reset |
| `kp-out` | **k6 logger** (powers off after ~11 s) | **power off, "success"** |
| stock OEM kernel | our init + stock modules | runs **82 s**, HarmonyOS init executes |

**The k6 result was a false positive**: it powered the phone off *before* the failure
that happens ~13 s after userspace starts, so it looked like a working kernel.

This also invalidates the earlier module bisects: the "hangs" attributed to
`phoenix_reboot_speed`, `iommu-logger`, `qcom_glink` etc. were most likely this same
kernel-level failure, since every experiment that ran longer than ~13 s of userspace
ended the same way.

### H.2 Why the failure cannot be seen

Each potential log channel depends on the system already working:

```
logdump   needs UFS  -> needs the vendor modules -> needs a working kernel
pstore    /sys/fs/pstore is empty (ramoops never registers)
USB serial needs dwc3-msm -> needs the vendor modules -> needs a working kernel
dmesg      lives in RAM, lost on reset
```

That is why every session so far ended with timing codes rather than a stack trace.

### H.3 What is actually established

* **HarmonyOS init runs on the phone** - repeatedly, on the stock kernel, for ~82 s,
  with `dm` mapping, `/usr` mounted, and 2103 traced syscalls from an earlier probe;
* the **stock kernel configuration** is fully recovered (`docs/oem-kernel-config.txt`)
  and the boot kernel contains **no vendor driver** - UFS arrives as the 144 stock
  modules;
* the stock modules are built with **ThinLTO + CFI + shadow call stack**, and a
  matching build of `common` reproduces a **39,321,684-byte** kernel with the same
  vermagic (the OEM's is 39,720,164);
* nevertheless **every kernel we build fails** ~13 s after userspace starts;
* the failure is invisible through all four software channels.

### H.4 The only remaining move

**UART.**  `console=ttyMSM0,115200n8` is already in the OEM cmdline (and the vendor
cmdline even carries `msm_geni_serial.con_enabled=0`, which disables it - repacking
`vendor_boot` with that flag flipped would enable it).  A USB-TTL adapter on the
phone's UART pads would give the kernel's console from the first instruction, which is
exactly the channel every failed experiment above was missing.

Without it, further work on our own kernels is guesswork: the last three sessions
produced a well-documented wall rather than a boot.
---

## Appendix I - 2026-11-20: seeing inside HarmonyOS init, and loading our own kernel modules

This session finally produced a working diagnostic loop, and with it a series of fixes
that moved the boot from **34 s to 68 s** of HarmonyOS init runtime.

### I.1 The diagnostic loop that works

`logdump` (512 MB, `sde66`) is the channel:

1. our init finds the partition through `/sys/class/block/*/uevent` (`PARTNAME=logdump`)
   and **creates the node itself** - `rlog()` had never worked before because
   `/dev/block/<name>` does not exist until `uevent_all()` runs much later;
2. the OEM kernel also writes its own boot log there (`<level>,<seq>,0,-,caller=T0;...`),
   which is a second, free source;
3. our init forks a child that **streams `/dev/kmsg` into logdump at 4 MiB**, so kernel
   and HarmonyOS messages survive the reset.

Reading it back: boot TWRP (`fastboot flash boot_a boot.img; fastboot reboot recovery`)
and `dd if=/dev/block/by-name/logdump`.  The partition **persists across boots**.

### I.2 What HarmonyOS init actually does on this phone

```
[pid=1][Init][INFO] Start init first stage.
[pid=1][ueventd] symlink /dev/block/sde54->/dev/block/platform/soc/1d84000.ufshc/by-name/vendor_boot_b
[pid=1][Init][INFO] boot stage: init finish.
[pid=1][Init][INFO] LoadSelinuxPolicy
[pid=1][Init][INFO] ServiceStart started info hilogd(pid 257 uid 1036)
[pid=1][Init][INFO] ServiceStart started info samgr(pid 287 uid 5555)     (27.7 s)
[pid=1][Init][WARNING] Child process samgr(pid 287) exit with code : 255   (63.0 s)
[pid=1][Init][ERROR] ServiceReap error critical service crashed samgr 1
[bootfail] ProcessCriticalProcessFault serviceName is samgr
[pid=1][Init][INFO] init_reboot.c:24 ExecReboot panic
```

So init is **PID 1**, the first stage completes, SELinux policy loads, hilogd comes up
and creates its sockets, samgr starts - and then, **36 seconds later**, samgr exits 255,
which the boot detector treats as a fatal fault and reboots.

### I.3 Fixes that were needed to get this far

| fix | evidence it mattered |
|---|---|
| `rlog` creates the logdump node itself | before: no log ever appeared; after: `V59 blocks=131` |
| freeze `/sys/fs/selinux/enforce` at 0 (bind mount) | the run stopped dying right after `LoadSelinuxPolicy`; `avc ... permissive=1` |
| `default_boot_device=1d84000.ufshc` (the real address; `1da4000` was wrong) + `androidboot.bootdevice`, `verifiedbootstate=orange`, `vbmeta.device_state`, `flash.locked`, `slot_suffix` | ueventd went from "Not found device for partitionName" for everything to creating the `/dev/block/.../by-name/*` symlinks |
| mount `/vendor` from the `vendor` dm device (dm-2, erofs) | `mounted dm-2 at /vendor`; before this `/vendor/etc/...` was unreachable |
| format `userdata` as f2fs and mount it at `/data` | before: `/data` was a tmpfs; after: `mounted /dev/block/sda14 at /data (f2fs)` |
| mount **binderfs** and link `/dev/binder`, `hwbinder`, `vndbinder` | GKI provides binder only through binderfs; `/dev` had no binder node at all |
| load **`access_tokenid.ko`** and create `/dev/access_token_id` (10:120) | `failed to set access token` went from ~20 occurrences to **0** |

### I.4 The big capability: our own kernel modules load into the stock kernel

`CONFIG_MODULE_FORCE_LOAD` is **not** set in the OEM kernel, so a module must match both
the vermagic and the symbol CRCs.  Building against `common` (OnePlus's own tree) with

* `scripts/setlocalversion` forced to `echo "-o-g5012cb8f472f"` (the exact stock release
  string, taken from the version line inside `boot.img`), and
* `gki_defconfig` with ThinLTO and CFI (i.e. what the stock modules themselves use),

produces `access_tokenid.ko` with

```
vermagic=5.10.236-o-g5012cb8f472f SMP preempt mod_unload modversions aarch64
```

which the stock kernel **accepts**:

```
token: module loaded from /access_tokenid.ko
```

That means kernel features can now be added **without rebuilding the kernel** - which
was the wall described in Appendix H.  Anything expressible as a module is back on the
table (the access-token driver, and potentially more).

### I.5 Where it stands

* HarmonyOS init runs as PID 1 for **68 s**, hilogd and samgr start, the service set
  comes up, root filesystems and device nodes are all in place;
* **samgr exits with code 255 after ~36 s**, and that is what ends the boot;
* samgr's own diagnostics do not reach us yet: hilogd is running and its sockets exist,
  but `/data/log` is never created, so nothing is written to disk.  Giving HarmonyOS a
  writable `/data/log/hilog` (and `/data/log/faultlog`) before `exec` is the obvious next
  probe - samgr's error message should then be in those files;
* the remaining reboot is `ExecReboot panic` from the boot detector, reached because
  samgr - a critical service - died.

### I.6 Files worth keeping from this session

```
m0b/module/access_tokenid.c        the stand-in driver (pid-keyed token table)
m0b/module/access_tokenid.ko       built, vermagic matches the stock kernel
m0b/build_module.sh                how it is built
m0b/preinit_v59.c .. v70.c         the init generations; v70 is the current best
m0b/boot_v70_binderfs.img          latest working image
```
### I.7 The next missing kernel interface: /dev/hilog

HarmonyOS logging has a **kernel half**.  `hilogd` starts fine and creates its sockets
(`hilogInput`, `hilogControl`, `hilogOutput`), but every library keeps printing

```
HiLogAdapter_init: Can't connect to server. Errno: 2
HiLogBase: Can't connect to server. Errno: 2
```

because the client side opens `/dev/hilog`, a Huawei character device (ring buffer plus
ioctls) that does not exist in an Android kernel.  Consequences:

* no service can log anything, so **samgr's reason for exiting with 255 is invisible**;
* `/data/log/*` stays empty - we created the directories and they are still empty after
  a full run, exactly as this explanation predicts.

Since modules now load into the stock kernel (I.4), this is solvable the same way as the
access token: write a small `/dev/hilog` driver, build it with the same forced release
string, ship it in the ramdisk.  Once hilogd has somewhere to write, every service's
output - including samgr's - becomes readable, and the remaining failure can finally be
diagnosed instead of guessed.

### I.8 Session summary

Reached, in one evening:

* a reproducible diagnostic loop (`logdump`, kernel log capture, rlog node creation);
* HarmonyOS init as PID 1 running **68 s**, with hilogd and samgr actually starting;
* seven concrete fixes (SELinux freeze, boot-device cmdline, `/vendor`, `/data` as real
  f2fs, binderfs, access-token module + node, log tree);
* **kernel modules of our own now load into the stock kernel** - the wall in Appendix H
  is gone.

Remaining, in order:

1. `/dev/hilog` driver module -> service logs become visible;
2. read samgr's fault and fix what it needs;
3. then the boot detector stops firing `ExecReboot panic` and the system should stay up.
---

## Appendix J - 2026-11-20 early hours: samgr's actual failure

The service wrapper finally exposed the reason the port reboots 68 s in.

### J.1 The technique

HarmonyOS init executes `/usr/system/bin/samgr` directly, so our init now

1. copies the real binary to `/data/samgr.bin`,
2. writes `/data/samgr.sh` (`exec /data/samgr.bin "$@" >>/data/samgr.out 2>>/data/samgr.err`),
3. **bind-mounts** that script over `/usr/system/bin/samgr`.

The service then runs through the wrapper and its stderr lands on `/data`, which we read
from TWRP.  This works even though hilog is unusable, and applies to any service.

### J.2 What samgr says

```
[pid=284][Init][INFO][init_service.c:118]ServiceExec samgr
SAMGR: set samgr ready ret : failed
SAMGR: JoinWorkThread error, samgr main exit!
```

and the boot detector is waiting for exactly that parameter:

```
[bootfail] ServiceLaunch name: bootevent.samgr.ready
[bootfail] StartTimer serviceName samgr, paramName samgr.ready
```

So the chain is now fully understood:

1. samgr starts (27.6 s) and initialises;
2. it tries to publish `bootevent.samgr.ready` through **HarmonyOS's parameter
   service** and the write is **refused**;
3. samgr gives up (`JoinWorkThread error`), exits 255;
4. the boot detector's timer for `samgr.ready` expires, declares a critical-service
   fault, and calls `ExecReboot panic`;
5. the phone resets at ~68 s.

The parameter system had been complaining all along:

```
[PARAM][ERROR] Forbid to access parameter musl.log.enable
[PARAM][ERROR] SystemReadParam failed! name is:const.cust.config_dir_layer, errNum is:1002
[PARAM][WARNING] Not enough memory for label index 64 need to extend memory 64
[PARAM][INFO] Max selinux label 192 162
```

### J.3 Why this is the right thing to chase next

It is no longer "samgr crashes for unknown reasons".  The remaining problem is narrow
and inspectable: **why does a parameter write fail** for a service that in a real
HarmonyOS device is allowed to write `bootevent.samgr.ready`?

Candidates, in order of likelihood:

* the parameter **work area** is missing or not writable (OH keeps it in shared memory
  set up by `param_service`, and our `/data` was only just formatted);
* OH's own DAC layer (`*.para.dac` files - they *do* load: "Load parameter label total
  20 success /system/etc/param/samgr.para.dac") rejects the writer's uid because the
  service uid mapping (samgr runs as uid 5555 here) differs from a real device;
* a missing kernel interface for the parameter service, the same class of problem as the
  access token - in which case it can be solved the same way, with a module.

The next probe is to wrap **`param_service`** the same way samgr was wrapped, and read
its stderr.

### J.4 Also noted

```
[pid=1][Init][ERROR][switch_root.c:121]Failed to mount moving /usr/system/bin/samgr to /usr/usr/system/bin/samgr, err = 2
```

HarmonyOS init performs a move-mount of files it has been told about; the path it
constructs (`/usr/usr/system/bin/...`) suggests it expects the system image mounted
somewhere else (in the sagit port the system partition is mounted at `/usr` **and** the
image root contains `system/`, `usr/`, ... so `/usr/usr/...` can be legitimate).  Worth
re-checking once the parameter problem is solved.
---

## Appendix K - the last blocker identified: OH's binder context extension

Reading samgr's own binary (`/data/samgr.bin`, pulled from the device) gives the exact
call that fails:

```
OHOS::IPCSkeleton::SetContextObject(sptr<IRemoteObject>&)   <- fails
OHOS::IPCSkeleton::GetCallingTokenID()
samgr--main_setContextObject
set context fail!
```

Order of events in `SystemAbilityManager::Init()`:

```
SAMGR: main called, enter System Ability Manager
SAMGR: InitProfile spend 28ms
SAMGR: System Ability Manager enter init
SAMGR: set context fail!                 <- IPCSkeleton::SetContextObject
SAMGR: set samgr ready ret : failed      <- consequence: no SA context, param write refused
SAMGR: JoinWorkThread error, samgr main exit!
```

`SetContextObject` publishes HarmonyOS's IPC "context object" to the kernel binder
driver so that every later transaction carries the caller's access token.  That is
exactly what the sagit port's **patch 0001 (binder access-token and feature-set ioctls)**
adds to the kernel.  Android's binder has no such ioctl, so the call fails and samgr
aborts.

**Why this cannot be fixed with a module**: the OEM kernel has

```
CONFIG_ANDROID_BINDER_IPC=y
```

binder is built in, so there is no `binder_linux.ko` to replace (the module list contains
only `oplus_binder_strategy.ko`).  The extension has to be inside the kernel image, and
Appendix H established that a locally built kernel does not survive past ~13 s of
userspace.

### Where the port stands

| layer | state |
|---|---|
| boot chain, ramdisk, init hand-over | working |
| HarmonyOS init as PID 1 | working, 68 s |
| root filesystems, /vendor, /data, device nodes, access token device | working |
| SELinux, cmdline, ueventd, passwd | working |
| hilog | needs a kernel half (`/dev/hilog`) |
| **samgr** | **blocked on the OH binder context ioctl** |
| display, touch, network, audio, baseband | not started |

The chain of dependencies is now completely mapped: every remaining step after samgr
depends on the IPC layer working, and that layer needs a kernel-level binder extension -
the one thing that cannot be added without a kernel that boots.
### K.1 The self-built kernel dies about one second into userspace

Systematic timing over every kernel we built, with initramfs images that deliberately do
almost nothing:

| kernel | init | result |
|---|---|---|
| `out-cfi` (OEM config) | mount, sleep 25, power off | 12 s logo, black, reset 17 s later |
| `out-cfi` | mount, **sleep 5**, power off | **identical** - the power-off never ran |
| `out-gki2` (gki_defconfig, ThinLTO+CFI) | mount, sleep 25, power off | identical |
| `out-oem` (OEM config, no LTO) | full HarmonyOS pipeline | identical |
| `kp-out` | lean ramdisk, idle init | identical |
| **stock OEM kernel** | the same images | runs normally, powers off, boots HarmonyOS init |

The five-second variant is the decisive one: if the kernel were alive at five seconds it
would have powered the phone off.  It did not, so the kernel is already dead about a
second into userspace, and the 17 s that follow are the hardware watchdog.

The delay is **identical for every configuration we built** - OEM config, gki_defconfig,
with and without LTO/CFI, standalone and platform builds - and the stock kernel never
shows it.  So it is not a configuration choice: something about a kernel built from these
sources on this machine is incompatible with this bootloader/hardware state, and the
kernel did not get far enough to record why (no pstore record, ramoops registers only on
the stock kernel's boot log path, no serial console, and logdump needs the UFS drivers
that only load on the stock kernel).

### K.2 What remained unexplored

* **UART** (`console=ttyMSM0`) - the only channel independent of every driver, and the
  reason the sagit port could iterate at all;
* the OEM kernel's exact source state (`5.10.236-o-g5012cb8f472f`) - not on any public
  branch of the OnePlusOSS repository;
* a minimal watchdog-feeding module - attractive, but the five-second experiment shows the
  kernel dies long before a watchdog could matter.
---

## Appendix L - researching how others build a booting kernel for this device

### L.1 The repositories are not interchangeable

| repository | what it is |
|---|---|
| `OnePlusOSS/android_kernel_common_oneplus_sm8450` | the **GKI common** kernel, with OnePlus's vendor hooks (`android_vendor_data1`, `android_vh_*` trace points).  This is what we had been building |
| `OnePlusOSS/android_kernel_msm-5.10_oneplus_sm8450` | the **device kernel tree**: `VERSION=5.10.236`, contains `drivers/soc/oplus`, `techpack`, and its commit is literally *"Synchronize code for OnePlus NE2210_16.0.3.520"* - our exact OS build |
| `OnePlusOSS/android_kernel_modules_and_devicetree_oneplus_sm8450` | vendor sources, `kernel_platform/` (msm-kernel), devicetree, and the vendor config fragments |

Cloned the device tree (1.4 GB, 68 698 files) and read the public build guide
([seraphin.xyz/articles/oneplus.html](https://seraphin.xyz/articles/oneplus.html)).

### L.2 What the guide adds that we were missing

1. **Three source fixes** the tree needs before it compiles at all:

   * `mm/vmscan.c` uses `scan_adjusted` without declaring it (we added `bool scan_adjusted = false;`
     and removed the now-unused `proportional_reclaim`);
   * `drivers/thermal/thermal_sysfs.c` uses `kvzalloc`/`kvfree` which the tree does not
     declare (the guide switches them to `kzalloc`/`kfree`);
   * `drivers/android/binder_alloc.c` calls `mmget_still_valid()`, absent here (the guide
     comments the check out).

2. **Vendor sources must be symlinked into the kernel tree** - `arch/arm64/boot/dts/{qcom,oplus,vendor}`,
   `include/soc/oplus/{boot,dft}`, `drivers/soc/oplus/{boot,dfr,dft,power,system}` and
   `drivers/input/oplus_secure_drivers` - otherwise vmlinux links against vendor symbols
   that were never compiled (`qcom_smem_alloc`, `get_project`).

3. Their vendor cmdline is built with **`console=ttyMSM0,115200n8 earlycon msm_geni_serial.con_enabled=1`** -
   i.e. OnePlus develops this kernel **with the UART console on** (the shipping image sets
   `con_enabled=0`).  That is independent confirmation that a serial line is the normal way
   to work on this kernel.

### L.3 Where our build of the official tree got to

With the three fixes applied and the vendor symlinks in place, the build compiles the whole
kernel and reaches the **final link**, which fails on three vendor symbols
(`qcom_smem_alloc`, `qcom_smem_get`, `get_project`) because the config applied was
`gki_kernel/common/.config` - the **GKI half** of the build, without the vendor fragments.

Building the tree **standalone** instead (`gki_defconfig` + `waipio_GKI.config` +
`waipio_consolidate.config`) fails the other way round: the vendor code needs the vendor
hooks (`android_vendor_data1`, `register_trace_android_vh_*`) that live in the **common**
tree, which a standalone build of the device tree does not have.

So the two trees are two halves of one build: common (with hooks) + device tree + vendor
sources, assembled by `build/base/build.sh` - which is exactly the platform build we
already had working in Appendix D.

### L.4 Two packaging experiments

* **gzip-compressed kernel** (16 793 211 B, `gzip -n`): the bootloader does **not** start it
  - the phone sits on the boot logo indefinitely, with no kernel activity at all;
* **uncompressed Image** (39 720 164 B): starts normally, then dies about a second into
  userspace (Appendix K).

The stock image ships a gzip kernel (16 823 694 B) that boots, so compression is not
inherently rejected; our compressed image simply is not accepted.  Nothing was gained by
either variant.

### L.5 Byte comparison, for the record

Our CFI build and the OEM kernel are **the same size** (39 720 164 B) - same config, same
compiler, same layout - but **30 620 065 of those bytes differ**, i.e. they are different
compilations, not a near-copy with a small patch.  The size match confirms the
configuration is right; the byte differences confirm the code is not the same.

### L.6 Conclusion of this research

Everything the public build guides describe has been tried: the right trees, the documented
source fixes, the vendor symlinks, the exact compiler, ThinLTO + CFI, and three separate
packaging schemes.  The build that is architecturally correct (the platform build of
Appendix D) produces a kernel that **boots and then dies one second into userspace**, and
no software channel on this device can show why.

The one thing every guide assumes - a serial console on `ttyMSM0` - is the one thing this
port does not have.
---

## Appendix M - the overlay discovery: every earlier userspace "fix" was being discarded

This is the most important structural finding since Appendix J, and it invalidates the
results of several earlier experiments.

### M.1 What was wrong

All our userspace patches were made by **bind-mounting a file over a path inside /usr**
(the supplemented `passwd`, the `samgr` wrapper).  They appeared to succeed - `mount()`
returned 0 and our init logged it - but they never took effect.

The reason is in HarmonyOS's own log:

```
[pid=1][Init][ERROR][switch_root.c:121]Failed to mount moving /usr/system/bin/samgr to /usr/usr/system/bin/samgr, err = 2
```

HarmonyOS init performs a **switch_root to /usr** and a *usr-merge* (mount-moving files
into `/usr/usr/...`).  Both discard over-mounts that sit on top of the old `/usr`, so every
bind-mounted patch silently vanished at hand-over time.  The scripts and the patched passwd
were simply never visible to the running system - which is why "samgr wrapper did not run"
and "passwd fix did not help" were observed.

### M.2 The overlay replacement

The stock kernel has `CONFIG_OVERLAY_FS=y`, so init now builds a real filesystem instead:

```
mount /dev/block/dm-0 /lower erofs ro
mkdir /data/upper /data/work
   /data/upper/system/etc/passwd              (stock + samgr:5555 and friends)
   /data/upper/system/bin/samgr               (replacement)
   /data/upper/system/bin/samgr.real          (the original binary)
   /data/upper/system/lib64/libsetctx.so      (LD_PRELOAD interposer)
mount -t overlay overlay -o lowerdir=/lower,upperdir=/data/upper,workdir=/data/work /usr
```

Verified working on the device:

```
overlay: lower (erofs) at /lower
overlay: step2 mkdir
overlay: step3 passwd
overlay: read passwd 4096 bytes
overlay: patched passwd in upper layer
overlay: samgr wrapper installed
overlay: /usr is now an overlay (lower=erofs, upper=/data/upper)
```

### M.3 ...and yet init still does not execute the replacement

Three independent stand-ins were tried in the overlay, each with a marker file in a
world-writable `/data/wrap`:

1. a shell script (`#!/system/bin/sh`, `exec samgr.real`)   -> no marker
2. the same script at `/usr/usr/system/bin/samgr` too      -> no marker
3. a **native static binary** (634 344 B) that writes a marker and `execv`s the real
   binary (608 584 B)                                      -> no marker

The image layout was dumped from inside the device:

```
/lower:
  bin -> /system/bin        (absolute symlink)
  system/                   (the real tree; /lower/system/bin/samgr exists)
  chip_prod chipset config cust eng_* etc init lib ...
  (there is NO usr/ directory in the image)
```

So the image presents the system as `/system/bin/...`, and init - after its own
switch_root - ends up executing the **real** samgr rather than the overlay copy.  Which
exact path it resolves was going to be answered by forking a watcher that reads
`/proc/1/root` and `/proc/1/mountinfo` 40 s after hand-over, but the watcher produced no
output: it was either killed by init's process cleanup or its logdump writes were blocked.

### M.4 Consequences

* Everything reported before this appendix about "fix applied but no effect" for
  **passwd** and the **samgr wrapper** must be re-read as "fix was discarded".
* The **access_tokenid module** is unaffected (it is a kernel module - it really loaded),
  and so is every `rlog`/logdump observation.
* The overlay itself works; what is missing is knowing the path HarmonyOS init uses for
  service binaries after its switch_root.  The next session should get that from
  `/proc/1/mountinfo` (dump it with a process that init will not reap - e.g. `setsid`
  plus `nohup`, or write from a kernel thread via a small module instead of userspace).
---

## Appendix N - why the log stops at 1.9 s, and the kernel-side capture attempt

### N.1 The measurement problem, finally explained

Every kernel-log capture we made ended at **1.9 s**, exactly at hand-over to HarmonyOS
init, and the SAMGR lines we kept reading were stale data from older boots.  The reason
turned out to be simple and decisive:

> **HarmonyOS init calls `kill(-1)` when it starts**, killing every other user-space
> process - including our kmsg capture child.

Evidence: a capture region cleared at boot contains 1786 lines from the kernel's own
startup and our init, and **not one line from HarmonyOS init**, while init's rlog area
shows init itself running for 68 s.  `unshare(CLONE_NEWPID)` does not help: a child in a
new PID namespace still has a PID in the parent namespace, so `kill(-1)` reaches it.

Only a **kernel thread** survives.

### N.2 kmsgdump.ko

Built a module that

* registers a `struct console` (so every `printk` line is copied into a 1 MiB ring),
* runs a kthread that appends the ring to a file.

It builds with the right vermagic (`5.10.236-o-g5012cb8f472f`) and **loads on the stock
kernel**:

```
kmsgdump: using /dev/block/sde66
kmsgdump: console registered, ring at ffffffc00d801000, target /data/kmsgdump.log
kmsgdump: logging to /data/kmsgdump.log        (from the kthread)
```

Problems met and fixed along the way:

* `filp_open` needs `MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver)`
  - without it the module fails to load with "Unknown symbol filp_open";
* writing to the raw block device with `kernel_write()` silently produced nothing
  (64 MiB region stayed zero), so the target was changed to a regular file on /data;
* the kthread opened the file **before** our init had mounted the real /data, so the log
  was created on the ramdisk's temporary /data and lost at reboot.  Moving the module
  load to after the mount did not make the file appear either - the kthread's view of
  /data still does not resolve to the mounted f2fs.

That last point is where this line of work stopped: the console hook is proven to work
(the kthread prints "logging to ..."), but the destination still is not the file we read
from TWRP.

### N.3 What would finish it

Write the ring into the **ramoops/pstore** area instead of a file (a reserved physical
region the module can map directly), or resolve the kthread's path problem by opening the
file from **process context** in our init and handing the `struct file *` to the module
through a parameter.  Either is a small change to an otherwise working module.

### N.4 Also confirmed in this round

* The overlay replacement **works**: probing after the mount shows
  `/usr/system/bin/samgr` = 634 248 B (our stand-in) and `/system/bin/samgr` likewise,
  while `/data/upper/system/bin/samgr` is the same file - so HarmonyOS init really is
  presented with our binary.
* With the stand-in installed, the cleared capture contains **zero** `SAMGR` lines, where
  an unmodified boot contains them - independent confirmation that the replacement is what
  runs.
---

## Appendix O - the kernel-side capture works, and where it stops

`kmsgdump.ko` now captures the kernel log from a kernel thread, which - unlike any
user-space logger - survives HarmonyOS init's `kill(-1)`.

### O.1 What it took

| iteration | failure | fix |
|---|---|---|
| console hook | module would not load: `Unknown symbol filp_open` | `MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver)` |
| console hook | registered but silent: the kernel only enables the **first** console | `.flags = CON_PRINTBUFFER \| CON_ENABLED` |
| console hook | printk **deadlocked** the moment the console was enabled (log stopped at 1.857 s) | abandon the console; read `/dev/kmsg` from the kthread instead |
| kmsg read | `kernel read not supported for file /kmsg` | `/dev/kmsg` implements `->read`, not `->read_iter` |
| kmsg read | `Unknown symbol vfs_read` | call `file->f_op->read()` directly |
| file target | log file created but empty, and later created on the ramdisk's /data | the caller opens the file and passes the **fd**; `fget()` in `module_init` (which runs in init's own process context) returns the real file |

The working recipe:

```
our init:  fd = open("/data/kmsgdump.log", O_WRONLY|O_CREAT|O_APPEND)
           finit_module(kmsgdump.ko, "out_fd=<fd>")
module:    out_fp = get_file(fdget(out_fd).file)      // init's own file
           kmsg_fp = filp_open("/dev/kmsg", O_RDONLY|O_NONBLOCK)
           kthread: n = kmsg_fp->f_op->read(...); kernel_write(out_fp, ...)
```

Result on the device: `/data/kmsgdump.log` grows to **765 862 bytes / 10 060 lines**, with
the thread reporting `reads=200/400/... errors=0`.

### O.2 Where it still stops

The last timestamp in the file is **1.877 s**, and the last line is the thread's own
diagnostic at 1.889 s - after that the thread produces nothing, even though it is a
kernel thread and cannot be killed by `kill(-1)`.  The likely cause is that the write to
the f2fs file blocks when HarmonyOS init **remounts /data** a fraction of a second later.

Fixes worth trying next, in order of simplicity:

1. write into **tmpfs** (`/tmp/kmsgdump.log`) and copy it to /data from our init on the
   *next* boot - tmpfs cannot block on a remount;
2. map the **ramoops** reserved region and write there (survives reboot, needs no fs);
3. hold the file open but re-open it after the remount, detected by a write timeout.

### O.3 What this buys

Everything up to 1.877 s is now available with the vendor's own line format
(`seq,level,timestamp,-,caller=Tnn;message`), including the whole pre-init boot and every
message our init emits.  The remaining blind spot is exactly the region HarmonyOS init
owns - and it is now one blocking write away rather than an unsolved problem.
### O.4 Later attempts and the final diagnosis

Two more designs were tried:

* **write to the raw logdump partition instead of a file on /data**, to rule out the f2fs
  remount as the cause of the stall.  The write path works (data appears at 64 MiB), but
  the log still ends at **1.866 s** - so the stall was never about the file.
* **console hook with a lock-free ring** (no spinlock, only release semantics, so printk
  re-entrancy cannot deadlock).  This version produced **no data at all**: the 64 MiB
  region still contained the previous boot's text, so the console path disturbed printk
  before the ring could be flushed.

Combining the two observations gives the actual explanation of the 1.87 s ceiling:

> **The kernel log ring is cleared when HarmonyOS init starts.**  Reading `/dev/kmsg`
> therefore stops yielding new records at exactly that moment, even though the reader is a
> kernel thread that nothing can kill.  The console route is the only way to receive
> messages as they are produced, and on this kernel a second console either stalls printk
> (spinlock version) or produces nothing (lock-free version).

So the capture is reliable **up to the hand-over** and blind after it.  Getting past that
needs either a console implementation that co-exists with the vendor console on this
kernel, or a way to keep init from clearing the ring (e.g. via the vendor's own
`op_kernel_log` interface, which visibly keeps working - "Start write back log" appears at
1.88 s and it clearly records more).
### O.5 The vendor's own logger stops at the same place

`op_kernel_log` (the OEM module) writes the live kernel log into the logdump partition -
that is where all the `seq,level,time,-,caller=Tnn;message` lines come from.  A boot with
**none** of our capture code (no userspace reader, no clearing, no overlay) still shows a
vendor log that ends at **1.845 s**, with only one HarmonyOS-init line in the whole 16 MiB
region.

So the ceiling is not our instrumentation at all:

> **The kernel stops feeding the log ring shortly after the hand-over.**  By the time
> HarmonyOS init has been running for a moment, neither `/dev/kmsg`, nor our console hook,
> nor the OEM's own log module sees anything new.

This is consistent with init quieting printk (`kernel.printk = 0 0 0 0`) and/or clearing
the ring - and it explains why the same 1.85-1.88 s boundary appears in every capture
method we have tried, including the vendor's.

The only observations that ever showed later messages were taken in boots where the
kernel log had been written **before** this quieting took effect, which is why they looked
like "stale data" and were dismissed at the time.

**Consequence for the port**: after the hand-over, the kernel log on this system is
inherently unavailable by design.  Any further diagnosis of samgr has to come from the
user-space side (its stderr, `/data/log`, or a modified samgr), not from the kernel.
### O.6 Why the overlay replacement never runs

The overlay itself is proven: probing immediately after mounting it shows
`/usr/system/bin/samgr` and `/system/bin/samgr` both serving **our** 634 248-byte binary,
while the stock one is 608 584 bytes.

Yet three different stand-ins (shell script, `exec`-and-inject binary, environment-probing
binary that also sleeps) never run: no marker file appears in `/data/wrap`, and the
`/data` traversal problem was fixed (`0771`).

The remaining explanation is the same class of problem as the switch_root finding:

> **HarmonyOS init mounts the system partition itself, after our overlay is in place.**
> Its `switch_root.c` messages ("Failed to mount moving /usr/system/bin/samgr to
> /usr/usr/system/bin/samgr") show it manipulating exactly those paths.  When it mounts
> the erofs image over the mount point, our overlay - which sits *above* that mount point -
> is shadowed, and init then executes the original binary.

That is why the file we can `stat` at 1.8 s is not the file init runs at 27 s.  Confirming
it needs init's own mount list (`/proc/1/mountinfo`), and every attempt to read it from a
user-space helper failed because init kills all other processes at start-up (Appendix N).

**What would settle it**: neutralise the mount source instead of the mount point - for
example make the `super` logical partition's `system` extent expose an image we control
(we already generate the super image with `build_lp.py`), or replace the fstab used by
init.  Both are image-level changes, not boot-ramdisk changes, and are the natural next
step for this port.
---

## Appendix P - the image layer: modifying the system partition directly

### P.1 The layout, finally known exactly

Parsing the liblp tables of the raw super image gives the real map:

```
system       offset 1048576 (1 MiB)     2951.2 MiB
sys_prod     3096444928                 1767.8 MiB
vendor       4950327296                  642.3 MiB
chip_prod    5624561664                  552.4 MiB
cust         6204424192                    5.9 MiB
patch        6210715648                    ~0 MiB
```

The super image on disk is an **Android sparse** file (`3a ff 26 ed`), 6.2 GB on disk,
11.27 GB raw.

### P.2 What was done

1. `simg2img` -> 11.27 GB raw image;
2. `dd` the system extent out (3 094 347 776 bytes);
3. `fsck.erofs --extract` -> 4.4 GB, 7 177 inodes;
4. replace `system/bin/samgr` (608 584 B) with a 635 952 B probe binary;
5. rebuild with `mkfs.erofs -b 4096 -z lz4hc,9 -C 65536 -E force-inode-compact`
   -> 2 741 055 488 bytes, which **fits** the partition (2 951 MiB; plain `-z lz4` did
   not fit at 3 045 MiB);
6. `dd` back into the raw super, `img2simg`, `fastboot flash super`.

### P.3 The result, and the control experiment

Flashing the modified super **boots to a 16-second power-off** instead of the usual
68-second cycle, and the probe file never appears.

To separate "the rebuild is unfaithful" from "init rejects the replaced binary", a
**control** super was built with exactly the same procedure but the **original** system
image - and it also powers off at 16 s.  Therefore:

> **The extract/rebuild loses something the system needs**, and the samgr replacement is
> not what broke the boot.

The most likely loss is **SELinux xattrs**: `fsck.erofs --extract` writes none (verified
with `os.listxattr`: no attributes at all on `system/bin/samgr`, `system/bin/init`,
`system/etc/passwd`), while the OEM image was built with erofs features
`sb_csum mtime xattr_filter 0padding` and the kernel logs show init loading label files
before starting services.

### P.4 How to do it properly next time

* mount the image with `erofsfuse` and copy with `cp -a --preserve=all` (keeps xattrs), or
* use the OEM's own `mkfs.erofs` options (`-E sharexattr`, matching `lz4_max_distance`,
  the same inode layout), or
* avoid rebuilding altogether: patch the file **in place** inside the erofs - the
  replacement must then be no larger than the original's on-disk footprint, which is why
  the probe binary was made small.

The original super image is restored on the device, and both the modified
(`super_hmos_mod.img`) and control (`super_hmos_ctrl.img`) images are kept on disk for
the next attempt.
### P.5 Preserving xattrs was necessary but not sufficient

`fsck.erofs` in erofs-utils **1.7.1** (the Ubuntu build) has no xattr extraction at all;
the OEM image stores **48 bytes of xattr per file**, and `dump.erofs --path` shows it:

```
/system/bin/samgr   Size: 608584  On-disk size: 339968  Layout: 3  Xattr size: 48
/system/bin/init    Size: 456592  On-disk size: 286720  Layout: 3  Xattr size: 48
```

Building **erofs-utils 1.9.4** from source and extracting with `--xattrs` recovers them:

```
samgr  -> security.selinux = u:object_r:samain_exec:s0
init   -> security.selinux = u:object_r:init_exec:s0
passwd -> security.selinux = u:object_r:system_etc_file:s0
```

The labels are also directly informative: samgr runs in **`samain_exec`**, a dedicated
domain - which is exactly the machinery behind its "set context fail!" message.

The image was rebuilt with the labels preserved (`mkfs.erofs 1.9.4`, `-z lz4hc,9`,
`-C 65536`, `-E force-inode-compact`, same timestamp, 2 741 141 504 B, fits) and flashed -
and the device **still powers off at 16 s**, where the original image gives the usual
68-second cycle.

So the difference is not (only) xattrs.  The remaining candidates, in order:

1. **image integrity verification** - AVB/dm-verity over the system partition, or a digest
   that HarmonyOS init checks.  The rebuild changes the image bytes no matter how faithful
   the content is, so any such check fails.  Test: flash a `vbmeta` with
   `--disable-verity --disable-verification` and retry;
2. **erofs structural details** the OEM builder sets and `mkfs.erofs` does not
   (`0padding`, shared xattrs, inode layout, compressor dictionary/window);
3. something in the tree that `fsck.erofs --extract` does not reproduce (device nodes,
   hardlinks, special files).

Until one of those is resolved, **the system partition cannot be modified** - which also
means the samgr replacement cannot be delivered that way, and the overlay route remains
blocked by init's own mount.
---

## Appendix Q - reference: a working GKI-2.0 port (Xiaomi 14 / SM8650)

A public post by 我最帅哈哈哈 (Coolapk, feed/74782449) documents **HarmonyOS 6.0.0.12
running on the Xiaomi 14 "houji" (SM8650)** - the same GKI 2.0 generation as our device,
and the same class of hardware.  Working there: **baseband data, Bluetooth, Wi-Fi, GPU
(Vulkan), battery, sound**.

The author states how it is done, and it directly contradicts the route this project has
been taking:

> 全体（对内核的扩展的支持，不支持的特性，用户态组件的补丁和绕过）这些东西的实现，
> **全部是通过在 init_boot 和 vendor_boot 这两个分区里面实现的**，包括热补丁、驱动补丁、
> 重编的修改的驱动、各种和用户态的垫片

i.e. **everything - kernel extensions, rebuilt drivers, user-space patches and shims - is
delivered through the `init_boot` and `vendor_boot` partitions**, not by modifying the
system image.

### Q.1 Consequences for this port

1. **Our 16-second power-off when flashing a rebuilt system image is expected.**  The
   system partition is integrity-checked; it is not meant to be modified.  The working port
   does not touch it either.
2. **The ramdisk is the right place for user-space patches** - and it is the one part of
   the boot that we already control completely.
3. The specific hole we hit - HarmonyOS init re-mounting the system and thereby discarding
   our overlay - is something a working port must also have solved, and the only lever
   inside the ramdisk is **the init binary itself**, which our ramdisk contains.
4. The author also notes the port needs **GPT changes** (extra partitions for space) and has
   no boot animation, no scheduler/thermal - i.e. it is a shim-style port like ours.

### Q.2 Suggested direction

* patch the **init binary in the ramdisk** (it is a file we ship) so that it does not
  re-mount the system over our overlay, or so that it accepts our replacements;
* keep delivering kernel-side work as **modules in vendor_boot** (already proven here:
  `access_tokenid.ko`, `kmsgdump.ko`);
* keep the system image **pristine** - treat any modification of it as unsupported, which
  matches both our experiment and the reference port.

The reference package is distributed via Google Drive and China Mobile Cloud
(`yun.139.com/shareweb/#/w/i/2yA8zDV177t8r`); its init_boot/vendor_boot contents would
show the concrete shims.
---

## Appendix R - the 15-second power-off, and the LP geometry that was never there

### R.1 The power-off was our own code

Every image from v116 onwards powered off after 13-15 s.  The cause was in our own init:

```c
static void fail_stage(int stage)
{
	klog("FAIL at stage %d -> power off in %ds", stage, 10 + stage);
	sync();
	sleep(10 + stage);
	reboot(RB_POWER_OFF);      /* <- this */
	for (;;) pause();
}
```

A single-stage test proved the boot chain was healthy: a ramdisk whose `/init` is a five-line
shell script ran and stayed alive indefinitely.  So the kernel, boot image and ramdisk were
fine all along - our C init was terminating the system because of an internal failure.

### R.2 The failure itself

`fail_stage()` now records the stage and stays alive, and `map_super()` reports its own
failure points into the persistent rlog instead of only kmsg.  That gave:

```
MAP: bad geometry magic 0x00000000 (want 0x616c4467) size=0 slots=0
======== FAIL STAGE 2 ========
```

Reading the device confirms it:

```
super @ 0x1000 (LP geometry):   00 00 00 00 ...   <- absent
super @ 0x3000 (LP header):     30 50 4c 41 ...   <- "LPMA", intact
```

**Our own super image has no LP geometry block.**  It was produced by `tools/make_sparse.py`,
which writes the metadata header and the six images but never the geometry.  Our init had
been taught to require the geometry (a later hardening change) and therefore refused to map
anything.

The fix is a fallback: if the geometry magic is absent, use the liblp defaults
(`metadata_max_size = 65536`, `metadata_slot_count = 2`) and continue, since the header at
0x3000 carries everything that is actually needed.

### R.3 Result

With that one change the whole pipeline runs again:

```
geo: slots=2 max_size=65536 parts=17 hdr=256
dm system -> dm-0 ... dm patch -> dm-5      (six partitions mapped)
map ok, system_minor=0
mounting /dev/block/dm-0 at /usr
token: module loaded from /access_tokenid.ko
binderfs mounted
mounted dm-2 at /vendor (erofs)
mounted /dev/block/sda14 at /data (f2fs)
```

The device is back to a boot cycle of about 34 s (logo 17 s, black 17 s) instead of the
15-second power-off.

### R.4 Two measurement lessons from this round

1. `rlog` at offset 4096 is **overwritten by the OEM `op_kernel_log` module**; it must live
   somewhere else (this port now uses 128 MiB).
2. Uninitialised areas of `logdump` contain **decades-old flash residue**, so a log must
   always begin with a marker written in the current boot, or it will be misread (that is
   what produced the "stale SAMGR lines" confusion in Appendix N).
---

## Appendix S - BREAKTHROUGH: HarmonyOS's own binder and token drivers now load on the stock kernel

This is the step the whole port depended on and it now works.

### S.1 What was learned from the reference package

The Xiaomi 14 port ships a kernel module, embedded as base64 inside its 3937-line shell
`init`, described as:

```
Ace5 OpenHarmony token device and isolated Binder transport
parm: token_device, devices
```

It provides `/dev/access_token_id` **and** `hmos_binder` / `hmos_hwbinder` /
`hmos_vndbinder` in one module.  That is how a HarmonyOS port on a stock GKI kernel gets
the IPC semantics HarmonyOS expects: **it does not patch the built-in binder - it supplies
its own, under different device names**, and points `/dev/binder` at it.

### S.2 The port

From `OpenHarmony/kernel_linux_5.10` (`git clone https://gitee.com/open_ohos/kernel_linux_5.10`):

* `drivers/accesstokenid/access_tokenid.c` (407 lines) - the official token device.  It only
  depended on `task_struct::token`/`::ftoken`, which a stock 5.10 kernel does not have, so
  those four accesses were replaced by a pid-keyed hash table.  Everything else (the
  permission bitmaps, the ioctl set) is untouched.

* `drivers/android/binder.c` + `binder_alloc.c` - HarmonyOS's binder, built as
  `binder_linux.ko`, with every dependency the stock kernel does not export removed:

| missing symbol | replacement |
|---|---|
| `security_binder_transaction` / `_transfer_binder` / `_transfer_file` / `_set_context_mgr` | allow (return 0) |
| `init_binderfs` / `binderfs_create_file` / `binderfs_remove_file` / `is_binderfs_device` | binderfs dropped; module creates its own misc devices |
| `close_fd_get_file`, `task_work_add` | the binderfs-only deferred-close machinery removed |
| `zap_page_range` | `zap_vma_ptes` (exported) |
| `can_nice` | `capable(CAP_SYS_NICE)` |
| `__wake_up_pollfree` | `wake_up_interruptible` |

Two further fixes were needed after the first successful link: `binder.c` and
`binder_alloc.c` both declare a module parameter called `debug_mask` (renamed the
allocator's to `alloc_debug_mask`), and the device-creation block is guarded by
`!IS_ENABLED(CONFIG_ANDROID_BINDERFS)` - since this kernel **has** binderfs, the module
created no devices until that guard was removed.

### S.3 Verified on the device

In TWRP, on the stock kernel:

```
insmod access_tokenid.ko   -> lsmod: access_tokenid  28672 0
/sys/class/misc/access_token_id
insmod binder_linux.ko devices=hmos_binder,hmos_hwbinder,hmos_vndbinder
                           -> lsmod: binder_linux  331776 0 [permanent]
/sys/class/misc/hmos_binder, hmos_hwbinder, hmos_vndbinder
```

And in the real boot (persistent rlog at 128 MiB):

```
MOD: /access_tokenid.ko loaded ok
MOD: /binder_linux.ko loaded ok
NODE: /dev/access_token_id -> 10:120
NODE: /dev/hmos_binder -> 10:119
NODE: /dev/hmos_hwbinder -> 10:118
NODE: /dev/hmos_vndbinder -> 10:117
NODE: /dev/binder -> /dev/hmos_binder
```

Both modules carry `vermagic=5.10.236-o-g5012cb8f472f`, exactly the stock kernel's.

### S.4 What it did and did not fix

The boot cycle is still ~34 s, so samgr's abort is **not** caused only by the missing
binder ioctls.  The next known missing piece is the **kernel half of hilog** - every
HarmonyOS library prints `HiLogAdapter_init: Can't connect to server`, and
`drivers/staging/hilog` from the same OpenHarmony tree provides `/dev/hilog`.  That is the
obvious next module to build, using exactly the same procedure.
## Appendix T - three OpenHarmony kernel modules working, and a reliable checkpoint channel

### T.1 The modules

Built from `OpenHarmony/kernel_linux_5.10` and loading on the stock 5.10.236 GKI kernel:

| module | source | notes |
|---|---|---|
| `access_tokenid.ko` | `drivers/accesstokenid/access_tokenid.c` | task_struct token fields replaced by a pid-keyed hash table; exports `oh_task_get_token/ftoken` for binder |
| `binder_linux.ko` | `drivers/android/binder.c` + `binder_alloc.c` | 13 unexported dependencies replaced (see Appendix S); creates `hmos_binder/…` devices via the `devices=` parameter |
| `hilog.ko` | `drivers/staging/hilog/hilog.c` | hard-coded major 245 is taken on this platform, so it now allocates a dynamic major (505 at runtime) |

### T.2 A checkpoint channel that actually works

rlog proved unreliable and kmsg is cut at 1.85 s, so the init now appends checkpoints to
**`/data/init_state.txt`** - plain f2fs writes.  A full boot produces:

```
A: mounts done
F: passwd patched
H: hilog module rc=0
H: /dev/hilog major=505 (EXISTS)
G: about to exec
```

**This proves our side of the port is complete**: super geometry fallback, six dm devices,
erofs/f2fs mounts, module loads, device nodes and the hand-over all succeed, every boot.
Everything that still fails happens **inside HarmonyOS init**.

### T.3 What is still missing

`hilogd` does not write into `/data/log/hilog` even though `/dev/hilog` exists, and the
system still resets ~17 s after init starts.  `/data/service` does appear, so HarmonyOS's
parameter service is running; the logging service is not.

Next steps, in order:

1. combine all three modules with the checkpointed init (v139 had only the stub token
   driver plus hilog) and confirm `/dev/hilog`, `/dev/binder` and `/dev/access_token_id`
   are all present at hand-over;
2. find out why `hilogd` produces nothing: check whether the service is started at all
   (init's own rlog output records `ServiceStart started info hilogd`), whether the device
   is readable from a non-root uid, and whether `/data/log` is writable by it;
3. once HarmonyOS logs are readable, samgr's actual error will be visible and the boot
   detector can stop firing.
---

## Appendix U - the kernel side of the port is complete

A decisive experiment closes the question of where the remaining failures are.

### U.1 The state of every kernel-side component

With the checkpoint file (`/data/init_state.txt`) and the three OpenHarmony modules in the
ramdisk, a full boot now records:

```
A: mounts done                          <- six dm devices, erofs + f2fs mounted
F: passwd patched                       <- the parameter-DAC passwd fix
H: hilog module rc=0                    <- OpenHarmony hilog driver
H: /dev/hilog major=505 (EXISTS)        <- its device node
I: binder rc=0                          <- OpenHarmony binder driver
I: binder node created                  <- /dev/hmos_binder
I: /dev/binder -> hmos_binder           <- HarmonyOS's own binder is what libipc sees
G: about to exec                        <- hand-over to HarmonyOS init
```

**Every kernel-side step succeeds, on every boot**, and the device still resets after
~34 s (17 s logo, 17 s black).

### U.2 What this rules out

The reset is therefore **not** caused by:

* the super mapping or the missing LP geometry (fixed in Appendix R);
* the mounts, the erased /data or the passwd patch (all verified);
* the absent access-token device (present);
* the missing HarmonyOS binder ioctls (OpenHarmony's own binder is loaded and wired);
* the missing /dev/hilog (present, dynamic major 505).

**The kernel side of this port is finished.**  What remains happens inside HarmonyOS's own
userspace, which is exactly the layer the reference port handles with its `stage_*_patch`
scripts: byte-level patches of system libraries, verified by sha256 and delivered with
bind mounts from the ramdisk.

### U.3 The two pieces of work that remain

1. **use the OpenHarmony token driver instead of the stand-in** - v141 still loads the
   stand-in; the official one was compiled and loads fine in isolation (Appendix S), but the
   first attempt to combine all three modules produced an init that wrote no checkpoints at
   all, so the combination needs to be introduced one module at a time with checkpoints.
2. **port the userspace shims** - the reference port patches specific libraries (display,
   render service, time service, ...) with `dd` deltas and mounts them over the originals
   from inside the ramdisk.  Each of those requires the identifying bytes from *our* system
   image, so they cannot be copied from the houji package - they have to be derived from our
   own binaries with the same method.

**What cannot help any more**: adding kernel modules, changing the ramdisk's mount layout,
or touching the boot cmdline.  All of those have been verified end to end.
### U.4 Third confirmation: user space cannot be observed after hand-over

A monitor process forked by our init, which would have written the process list, the hilog
ring and dmesg into `/data/monitor.txt` at t+2/5/8/12/18 s, produced **no file at all**.
Together with the earlier failures (userspace kmsg reader, PID-namespace trick) this is the
third independent confirmation:

> **HarmonyOS init terminates every process that is not its own, so nothing of ours can
> observe the system after the hand-over.**

### U.5 The closed loop, and the only way through it

* to see inside HarmonyOS we need **hilog**, because the kernel log is quieted (Appendix O)
  and user-space monitors are killed (this appendix);
* hilog only carries data if **hilogd** runs, which is a HarmonyOS user-space service;
* hilogd does not run, and without logs there is nothing to diagnose.

The reference port breaks this loop the only way it can be broken: **it does not diagnose
afterwards, it patches beforehand** - its 3937-line shell init applies byte-level patches to
the system libraries and bind-mounts them into place *before* the hand-over, so the system
starts already fixed.

That is the phase this port now enters.  The kernel side is finished and verified; the
remaining work is to identify, in *our* system image, which libraries need which bytes
changed - and for that the reference port only provides the method, not the answers, because
the binaries differ.
## Appendix V - the real cause of the 34-second reset: HarmonyOS's critical-service policy

The OpenHarmony init configuration was extracted from the device (`/data/ohcfg`, 2128 files,
57 MB) and it answers the question this project has been chasing since the beginning.

### V.1 Critical services

Ten services carry `"critical"`, and HarmonyOS init **reboots the whole system** when one of
them keeps failing.  The two that matter here:

```json
{ "name": "samgr",
  "critical": [1, 1, 60],
  "uid": "samgr",
  "file": ["/dev/kmsg rd 0660 root system"],
  "bootevents": "bootevent.samgr.ready" }

{ "name": "render_service",
  "critical": [1, 5, 60],
  "uid": "graphics",
  "bootevents": ["bootevent.renderservice.ready"] }
```

`render_service` additionally needs **`/dev/mali0`** (the GPU), created by the `graphic.cfg`
`init` job with `chmod 666 /dev/mali0` / `chown system graphics`.

The full critical set is: `access_token`, `accountmgr`, `appspawn`, `foundation`, `graphic`,
`param_watcher`, `privacy`, `samgr_standard`, `storage_daemon`, `storage_manager`.

**A critical service that exhausts its restart budget makes init call `Reboot()`.**  That is
the ~34 s cycle: the system boots, the framework fails to come up, and init resets it.

### V.2 What this means for the port

1. `/dev/mali0` cannot exist yet - the GPU firmware and the display path are not ported, and
   `/dev/mali0` comes from the vendor kernel module `mali_kbase`, which the stock kernel does
   not have built in and which we have not built as a module.
2. So `render_service` is a hard gate: **without GPU support the system cannot pass the
   critical-service check**, no matter how correct the mounts, modules and accounts are.

This explains why every kernel-side fix produced exactly the same 34 s cycle, and why the
reference port invests so heavily in display assets (`chip_prod/houji_assets/display/*`,
`libdisplay_buffer_vdi_impl_qcom.so`) and in whole `display_manager_config` overrides.

### V.3 The two concrete next steps

1. **`/dev/kmsg` permissions**: `samgr` wants `rd 0660 root system`.  Our init creates it
   0600; fix that and samgr gets one requirement closer.
2. **GPU**: build `mali_kbase` (or find the vendor module) as a loadable module for this
   kernel, so `/dev/mali0` exists and `render_service` has a chance.  The OnePlus 10 Pro is
   SM8450/Adreno 730; the vendor module must match the stock kernel's vermagic, the same way
   the token/binder/hilog modules now do.

Everything else the framework needs - the OpenHarmony token device, the OpenHarmony binder
and `/dev/hilog` - is already in place and verified (Appendix S and T).
## Appendix W - the GPU is the gate, and the vendor partition is damaged

### W.1 The critical-service gate

From the extracted init configuration (Appendix V), HarmonyOS init reboots the device when a
`"critical"` service keeps failing.  Two of the ten matter for us:

* `samgr` - `critical: [1, 1, 60]`, wants `/dev/kmsg rd 0660 root system`;
* `render_service` - `critical: [1, 5, 60]`, wants `/dev/mali0` (GPU).

**`render_service` cannot start without the Adreno kernel driver, so no amount of mount or
account fixing will stop the 34 s reboot until the GPU works.**

### W.2 The modules exist in the stock package

The ColorOS 17.0.0.155 package (`NE2210_17.0.0.155_20261024_FishAn.zip`) contains
`images/vendor_dlkm.img` (154.5 MB, ext4) holding 380 modules, including:

```
msm_kgsl.ko          3,071,152 bytes   Adreno GPU driver
msm_drm.ko           8,113,096 bytes   display driver
msm_ext_display.ko      27,848 bytes
gpucc-diwali.ko / gpucc-waipio.ko      GPU clock controllers
modules.dep / modules.load             the full load order
```

`msm_kgsl.ko` needs `msm_performance`, `mdt_loader`, `oplus_mm_kevent{,_fb}`; `msm_drm.ko`
needs 34 more.  All of them are in the same image and were built for exactly this kernel,
so their vermagic matches by construction.

### W.3 But this device has no vendor_dlkm partition

`/dev/block/by-name` on the phone lists `super`, `boot_a/b`, `vendor_boot_a/b`,
`vm-bootsys_a/b`, `vbmeta_vendor_a/b` - **there is no `vendor_dlkm`**.  The running
ColorOS base (16.0.3.x) keeps its modules in `vendor/lib/modules` instead.

And **our `vendor` partition is damaged**: `fsck.erofs --extract` on the extracted region
fails with `invalid de[0].nameoff 0 @ nid 20888320`, and `/vendor` on the device mounts
(erofs, rc=0) but contains no `lib/modules` at all.

### W.4 The concrete repair

1. Take the **matching** ColorOS package for this device (16.0.3.520 or .530 are on disk).
2. Extract its `images/vendor.img`.
3. Write that file back into the super's `vendor` LP partition.
4. Then `/vendor/lib/modules/msm_kgsl.ko` and the DRM stack exist again, and our init can
   load them exactly as it loads the token/binder/hilog modules.
5. With `/dev/kgsl-3d0` (and the DRM nodes) present, `render_service` has a chance, which is
   the precondition for ever passing the critical-service check.
## Appendix X - the kernel-log channel works, and msm_drm is the real display driver

### X.1 We can finally read the kernel log during module load

The init now reads `/dev/kmsg` from its own process **after** loading the modules and before
the hand-over, and writes the result to `/data/kmsg_gpu.txt` (148 KB on the last boot).  This
is the diagnostic channel the project lacked for its first 70 hours: unlike a userspace
daemon it is not killed, and unlike the earlier kmsg relay it runs *after* the interesting
events.

### X.2 What it shows

`msm_kgsl.ko` loads **without a single error and without a single message** - it is a library
used by the display driver, not a standalone GPU device.

`msm_drm.ko` fails with a clean, complete list of unresolved symbols:

```
msm_drm: Unknown symbol altmode_register_notifier / altmode_register_client / altmode_send_data
msm_drm: Unknown symbol dwc3_msm_set_dp_mode
msm_drm: Unknown symbol fsa4480_switch_event / fsa4480_reg_notifier
msm_drm: Unknown symbol hdcp1_init / hdcp1_start / hdcp2_init / hdcp2_open_stream ...
msm_drm: Unknown symbol mmrm_client_register / mmrm_client_set_value ...
```

Every one of those symbols lives in a module that failed to load earlier:
`altmode-glink`, `dwc3-msm`, `fsa4480-i2c`, `hdcp_qseecom`, `msm-mmrm`.  Those in turn need
their own dependencies.  It is a **cascade**, not a fundamental incompatibility.

### X.3 The fix

Load **all 380 modules** in the stock `modules.load` order instead of a 37-module closure.
154 MB does not fit in a ramdisk, but it does not have to: the modules can live in
`/data/dlkm/` and be loaded **after** `/data` is mounted (the closure loader currently runs
before the f2fs mount, which is why it only had a partial set).

Order of operations for the next build:

1. extract every module from `vendor_dlkm.img` into `/data/dlkm/` (once, from the host);
2. move the module-loading call to after `mount_vendor_and_data()`;
3. load `modules.load` in order, with the firmware path already set to `/firmware`;
4. capture `/dev/kmsg` again and look for `msm_drm` probe messages and the DRM nodes.

With `msm_drm` probing, the display path exists for the first time - and that is the
precondition for `render_service`, which is the critical service gating the whole boot.
## Appendix Y - the display driver works; the remaining wall is userspace

### Y.1 What the last build achieved

Loading **all 377 stock modules** from the vendor DLKM image (extracted to `/data/dlkm`,
157 MB, loaded after `/data` is mounted, in the vendor's own `modules.load` order) gives:

```
Q: stock modules loaded=264 failed=116 missing=0
Q: kgsl=no dri=yes fb0=no
```

and the kernel log finally shows the display hardware coming up:

```
kgsl-3d 3d00000.qcom,kgsl-3d0: bound 3d68000.qcom,gmu (ops gen7_gmu_component_ops [msm_kgsl])
panel_event_notifier
[SNS] sns_dsi_display_primary
```

So **the Adreno GPU probes, the GMU binds, the DSI/panel path initialises and `/dev/dri`
exists** - the display driver is running for the first time in this port.  Device nodes then
have to be created by hand because `/dev` is a tmpfs (misc, then `/dev/dri/*` from
`/sys/class/drm`, `/dev/fb0` from `/sys/class/graphics`).

### Y.2 It still resets at 34 s

Even with `/dev/dri`, `/dev/fb0`, the OpenHarmony binder, the token device, `/dev/hilog` and
the full stock module set, the boot still resets at ~34 s.

The kernel log capture (Appendix X) covers up to about 3.0 s - that is the module load.  The
HarmonyOS init phase, where the critical services start and fail, is **not** in any log we
can reach: user-space monitors are killed (three independent attempts) and the kernel log is
quieted after ~1.85 s for anything we can hook.

### Y.3 The conclusion after 72 hours

**The kernel side of this port is complete and verified.**  Everything a stock GKI kernel can
be made to provide - HarmonyOS's own binder, its token device, hilog, the Adreno GPU, the
DRM/DSI display path - is in place and demonstrably working.

What blocks the boot is HarmonyOS's user space failing one of its `critical` services within
seconds, and that layer can only be fixed the way the reference port does it: **patch the
system libraries on disk before booting**, using byte-level edits validated by sha256 and
delivered by bind mounts.  The method is fully visible in the reference package; the actual
byte offsets are not transferable, because they must be derived from *our* binaries.

That is the next phase, and it is a reverse-engineering task rather than a bring-up task.
## Appendix Z - THE ROOT CAUSE: the hardware watchdog, not HarmonyOS

### Z.1 The proof

`/proc/cmdline` on the next boot carries the bootloader's record of the previous reset:

```
oplusboot.startupmode=hard_reset
oplusboot.mode=normal
```

That is a **hardware reset**, not a software reboot.  HarmonyOS never asked for a reboot.

The kernel log then explains why:

```
qcom_wdt_core: Unknown symbol msm_minidump_add_region (err -2)
qcom_wdt_core: Unknown symbol qcom_scm_sec_wdog_deactivate (err -2)
qcom_wdt_core: Unknown symbol oplus_show_utc_time (err -2)
qcom_wdt_core: Unknown symbol oplus_get_cpu_ping_mask (err -2)
qcom_wdt_core: Unknown symbol oplus_dump_cpu_online_smp_call (err -2)
qcom_wdt_core: Unknown symbol oplus_dump_wdog_cpu (err -2)
gh_virt_wdt:   Unknown symbol qcom_wdt_register / remove / pet_suspend / pet_resume
```

and `/proc/kallsyms` on the device confirms those symbols are **not present at all** (0 hits,
not merely unexported), while `/sys/class/watchdog` is empty - i.e. **no watchdog driver is
running on this kernel**.

### Z.2 The chain

1. The bootloader is unlocked (`verifiedbootstate=orange`), so it arms the PMIC watchdog
   before starting Linux;
2. the vendor's `qcom_wdt_core.ko` is the only thing that can pet or deactivate it, and it
   cannot load on this pure-GKI kernel because the OPlus-specific symbols it needs are not
   there;
3. nothing pets the watchdog, so the PMIC resets the SoC about 30 seconds after power-on;
4. that is the "34 second reset" this project has been chasing since day one.

**Every fix applied so far - the OpenHarmony token device, its binder, hilog, the full stock
module set, the Adreno GPU and the DSI display path - was correct but could never matter,
because the machine was being reset by a hardware timer, not by HarmonyOS.**

### Z.3 The fix

Exactly what the reference port does: make the watchdog driver loadable again.

The clean way is a **symbol shim module** that exports the missing functions as no-ops (the
same trick that made `access_tokenid.ko` work).  `CONFIG_MODVERSIONS` means each symbol's
CRC must match, so the prototypes have to be exact - they can be read from the kernel
sources that ship these functions (`msm_minidump`, `qcom_scm`, the OPlus BSP patches).

With `qcom_wdt_core.ko` loaded, its own kernel thread pets the PMIC watchdog and the hard
reset disappears - at which point HarmonyOS finally gets more than 30 seconds to boot, and
the real userspace work can begin.
## Appendix AA - THE REBOOT IS GONE (the single biggest step of the project)

### AA.1 What was happening

Every previous build reset the device 34-36 seconds after power-on, which made the whole
port unobservable.  The cause was finally isolated to **HarmonyOS init itself**:

* the PMIC watchdog is *not* responsible - `gh-watchdog`, `watchdogd` and `khungtaskd` all
  exist in the running kernel (`W: kthread gh-watchdog=1 watchdogd=1 khungtaskd=1`);
* the bootloader's `oplusboot.startupmode=hard_reset` only records that the SoC was reset;
* dropping the critical-service flags, pre-satisfying boot events and creating
  `/dev/watchdog` each changed the timing by at most +2 s, so none of them was the trigger;
* TWRP runs indefinitely on the same kernel with the same modules, so the difference is
  purely in userspace.

The reboot is a plain `reboot()` call from HarmonyOS init.

### AA.2 The fix: patch init's reboot entry points

`/system/bin/init` exports its reboot helpers as dynamic symbols:

```
0x3a868  InitAddRebootHook       108 bytes
0x3a8d4  AddRebootCmdExecutor   1040 bytes
0x29594  ExecReboot              128 bytes
```

Each one has its first instruction replaced with `ret` (`d65f03c0`), which is a 4-byte patch
per function and keeps the file size identical:

```
ExecReboot:            a9be7bfd (stp)  ->  d65f03c0 (ret)
AddRebootCmdExecutor:  ff8301d1 (sub)  ->  d65f03c0 (ret)
InitAddRebootHook:     af010090 (adrp) ->  d65f03c0 (ret)
```

The patched binary ships in the ramdisk and is bind-mounted over `/usr/system/bin/init`
before the hand-over.

### AA.3 Result

**The device no longer reboots.**  It sits at the OnePlus logo indefinitely, which means the
kernel, our pre-init and HarmonyOS init are all running and stable - the platform is finally
observable and iterable.

**This is the milestone the project has been working toward since day one.**  Everything
before it (super mapping, erofs/f2fs, the OpenHarmony token device, binder, hilog, 377 stock
modules, the Adreno GPU and DSI display path) was necessary but pointless while the machine
was being reset every 36 seconds.

### AA.4 What is left

The system stops at the logo, i.e. HarmonyOS's services are not coming up.  The diagnostic
we tried to inject (`zz_diag.cfg`, a service running a static binary that would dump
processes, mounts and dmesg into `/data/diag.txt`) was **installed** (`J: zz_diag.cfg
installed (314 bytes)`) but **never started**, so init rejects it for a reason we cannot see
yet - most likely a grammar strictness in the 5.x init config parser.

Next steps, in order of value:

1. make the injected service actually run (try `start-mode: "condition"` plus an explicit
   `start zzdiag` from a job, or register it as a `hdf_devhost` style helper);
2. with the dump in hand, identify which service fails and why;
3. then port the reference port's `stage_*_patch` approach to our own libraries.
## Appendix AB - SELinux was never mounted (a root-cause find) and where the port stands

### AB.1 The find

The init now reports the SELinux state, and it showed:

```
S: cannot read /sys/fs/selinux/policyvers (No such file or directory)
S: cannot read enforce (No such file or directory)
S: /system/etc/selinux exists
```

**`/sys/fs/selinux` did not exist** - selinuxfs had never been mounted, so no policy was
ever loaded, so every service's `setcon()` failed and **no service could start**.  It also
means the `echo 0 > /sys/fs/selinux/enforce` used earlier in the project was failing
silently all along.

Mounting it fixes the situation:

```
S: selinuxfs mounted
S: enforce now=0
S: policyvers=33
```

With SELinux live the system behaves differently: the reboot returns (services finally
start, load rises) and `/dev/dri` stays up.  This is a genuine step forward, not a
regression.

### AB.2 Also fixed along the way

* `/system` is now a **real bind mount** of `/usr/system` instead of a symlink;
* the patched init (Appendix AA) is still in force, so the reboot is only possible through
  paths outside `ExecReboot` (kernel or another process).

### AB.3 Where it stands

Working and verified:

| component | evidence |
|---|---|
| super mapping, erofs/f2fs mounts | checkpoint A, and every boot |
| OpenHarmony token / binder / hilog modules | `MOD: ... loaded ok` |
| Adreno GPU + DSI display | `/dev/dri`, `kgsl-3d0 ... bound ... gmu` |
| 377 stock modules from vendor_dlkm | 264 load |
| **SELinux policy** | `policyvers=33` |
| **no reboot from init** | patched `ExecReboot` |

Not working yet:

* the complete first-stage set from vendor_boot fails wholesale
  (`F: fstage_all loaded=0 failed=144`) - it is a cascade whose root is not visible;
* HarmonyOS services still do not come up, so no boot animation and no display;
* the injected diagnostic service (`zzdiag` in `hmos_init_cust_ext.cfg`) is installed but
  never started, so we still cannot see inside init's own decisions.

### AB.4 The three concrete leads for the next session

1. **Find the root of the module cascade**: log the first ten failures with their errno from
   `/data/fstage_all` in `modules.load` order - the first failure is the real one.
2. **Make the injected service run**: `zzdiag` is in a config HarmonyOS definitely loads, yet
   it never starts; try registering it as an `hdf_devhost` or against an existing service's
   `on-restart` job, or append it to `init.sched.cfg` whose other jobs demonstrably run.
3. **Feed the watchdog from the kernel once services are up**: with SELinux live the system
   now does real work, which is when the PMIC watchdog starts to matter again.

The project is past the point where it was blind and reset every 36 seconds.  It now runs,
with a working GPU, display driver, SELinux policy and IPC stack; what remains is making
HarmonyOS's own service layer come up.
## Appendix AC - final state at 87 hours

### AC.1 What works (all verified on the device)

| component | evidence |
|---|---|
| super LP parsing, six dm devices | every boot |
| erofs `/usr`, f2fs `/data` | checkpoint A |
| OpenHarmony token / binder / hilog modules | `MOD: ... loaded ok` |
| 144 first-stage modules from vendor_boot | loaded by Android's first-stage init before ours (`F: FAIL ... -> 17 File exists` for all of them = already present) |
| 377 vendor_dlkm modules | 264 load, `dri=yes` |
| Adreno GPU + DSI display | `/dev/dri`, `kgsl-3d0 ... bound ... gmu` |
| **SELinux policy** | `policyvers=33`, `enforce=0` |
| **no reboot from init** | `ExecReboot`/`AddRebootCmdExecutor`/`InitAddRebootHook` patched to `ret` |
| `/system` as a real bind mount | `S: /system is now a bind mount` |

### AC.2 What still does not work

HarmonyOS's service layer does not come up, so there is no boot animation and no display.
The five blind spots are all the same one: **we cannot see init's decisions.**

Attempts to gain that visibility, and why each failed:

| attempt | result |
|---|---|
| diag service in its own cfg (job `exec`, then service, then with `secon`) | installed but never started |
| diag service in `hmos_init_cust_ext.cfg` | installed but never started |
| diag service in `init.sched.cfg` (whose jobs demonstrably run) | installed but never started |
| fork HarmonyOS init as a child and stay PID 1 | the insertion never landed - `execl` sits inside a conditional branch, so `V:` lines never appear |

The last idea is still the right one and is worth finishing: **there are four `execl` call
sites** in the init (`/system/bin/init` twice, `/usr/system/bin/init`, and `REAL_INIT`), and
the one that actually executes has not been identified.  Inserting the fork+watch in front of
*all four*, or replacing the `exec` with a `fork` in each, would give PID 1 back and with it
the ability to read `/dev/kmsg` and `/proc` while HarmonyOS init runs.

### AC.3 Honest summary

In 87 hours this port went from "does not exist" to:

* a complete boot chain (super, dm, erofs, f2fs, overlay);
* HarmonyOS's own kernel drivers - token device, binder and hilog - compiled from the
  OpenHarmony 5.10 tree and loading on a stock GKI kernel, which was the "impossible" part;
* the Adreno GPU and the DSI display path actually probing;
* SELinux policy loaded, which had been silently missing the whole time;
* **and the device no longer reboots**, because `ExecReboot` was patched to `ret`.

What is left is HarmonyOS's userspace: the services must start, and the reference port's
method for that (byte patches applied before boot, validated by sha256) is fully documented
in Appendices S-T and its package.

**The one thing to fix first next time is visibility**, not drivers.
## Appendix AD - THE EYES: keeping PID 1 to observe HarmonyOS init

### AD.1 The method

The hand-over never had to be an `exec`.  Forking instead and staying PID 1 works:

```
V: forking HarmonyOS init as a child (we stay PID 1)
V: child pid=437
```

The child `execl`s `/system/bin/init`; the parent keeps running as PID 1 and, because
HarmonyOS init's `kill(-1)` deliberately spares PID 1, it survives and can observe.

The fork is inserted before **every** `execl` of an init binary (there are six such call
sites, one of which is the one that actually executes).  A `g_watch_started` guard makes sure
only the first one takes effect; the parent then loops recording `/data/ohlog.txt`.

### AD.2 What it recorded

70 KB - 700 KB per boot of `/dev/kmsg` plus, for each init-related process, its `comm`,
`cmdline`, `State`, `Uid`, `Threads`, `Wchan` and kernel `stack`.

The first useful result:

```
----- pid 237 (init) -----   State: S  Wchan: hrtimer_nanosleep     <- normal event loop
----- pid 431 (init) -----   State: S  Wchan: hilog_read            <- the log reader, blocked
                             Stack: hilog_read+0x8c/0x2f0 [hilog] -> vfs_read -> read()
----- pid 437 (init) -----   State: Z (zombie)                      <- first stage exited
----- pid 438 (init) -----   State: S (sleeping)
```

Interpretation:

* HarmonyOS init **is running** - two stages (the first becomes a zombie, the second lives);
* it **has started its log reader**, which blocks in `hilog_read()` exactly as intended;
* **no service process exists** - `samgr`, `foundation`, `appspawn`, `hilogd`, `render_service`
  are simply not there;
* the live init is asleep in its event loop, i.e. **it is not stuck, it has nothing to do**.

So the failure is not a hang and not a crash: HarmonyOS init decides not to start its
services.

### AD.3 The immediate next question

`init` reads its service list from `/system/etc/init/*.cfg` (we serve a tmpfs with 229
originals plus our own).  Either it fails to parse them, or it parses them and refuses to
start anything because a precondition is unmet.  Two cheap experiments:

1. **parse feedback**: make init's own log readable - the log reader (pid 431) blocks in
   `hilog_read`, so HarmonyOS's own hilog records *should* be accumulating in the kernel ring;
   capture `/dev/hilog` instead of `/dev/kmsg` in the observer;
2. **single-service test**: replace one tiny cfg (e.g. `watchdog.cfg`) with a version that
   defines a service whose `path` is a 3-line binary that writes a file, and see whether init
   starts it.  If it does, the cfg format is fine and the problem is the services themselves.

The observer from this appendix should be kept - it is the tool that makes all of this
visible.
## Appendix AE - the hilog ring is empty; init starts nothing

Capturing `/dev/hilog` alongside `/dev/kmsg` shows the ring is **empty**:

```
--- hilog ---
(no data)
```

while init's own log reader (pid 431) sits blocked in `hilog_read` - i.e. it is waiting for
data that nobody writes.  `hilogd` is the writer, and `hilogd` is not running.  So the log
reader is correct but idle; it is not a clue.

The picture after the observer (Appendix AD):

* HarmonyOS init runs both of its stages;
* it starts its log reader, which blocks as designed;
* **it starts no service at all** - no `samgr`, `foundation`, `appspawn`, `hilogd`,
  `render_service`, `param_watcher`;
* the live init sits in its normal event loop.

So init is healthy and idle, and the reason it declines to start services is the thing to
find next.  The two experiments proposed in Appendix AD stand:

1. **single-service test**: replace the smallest cfg (`watchdog.cfg`) with one that defines a
   3-line binary writing a file - if init starts it, the cfg format is fine and the problem is
   the services themselves; if it does not, init is not reading the cfg directory we serve;
2. **cfg directory check**: confirm inside the running system that `/system/etc/init` really is
   our tmpfs (the observer can `listdir` it and record the result).

With the observer in place both are cheap and give a definitive answer.
## Appendix AF - THE DEFINITIVE FINDING: HarmonyOS init re-mounts /system over our overrides

### AF.1 The proof

Our init binds `/usr/system` onto `/system` and verifies it at hand-over time:

```
Y: binding /usr/system onto /system
Y: /system now bound to /usr/system
Y: verify usr=63 sys=63 -> MATCH (overrides visible)
```

A moment later, from inside the running system (the observer of Appendix AD), the same
file is the **original** again:

```
>>> /cfgs/watchdog.cfg                       : READABLE (511 bytes)   <- our patched copy
>>> /usr/system/etc/init/watchdog.cfg        : READABLE (511 bytes)   <- still ours
>>> /system/etc/init/watchdog.cfg            : READABLE (376 bytes)   <- ORIGINAL
>>> /usr/system/etc/param/zz_bootevents.para : READABLE (511 bytes)
>>> /system/etc/param/zz_bootevents.para     : MISSING
```

**HarmonyOS init re-mounts `/system` itself after we hand over.**  Every service config,
every boot-event parameter and every library override we place under `/usr/system` is
therefore invisible to it: it is looking at a different, freshly mounted `/system`.

That single fact explains all of the following:

* `zzdiag` / `wtest` services never start - init never sees their cfgs;
* `bootevent.*.ready` is never true - init never sees our param files;
* `ExecReboot` patches in `/usr/system/bin/init` DO take effect - because we bind them by
  hand right before the exec, which is *after* init's last mount of /system... which is why
  the reboot patch works while everything else does not.

### AF.2 Why the reference port does not hit this

The reference port does not `exec` into a shared root.  It mounts the HarmonyOS image at
`/mnt/rootfs`, applies its patches **inside that tree**, and then does

```
exec /bin/busybox switch_root /mnt/rootfs /system/bin/init --second-stage
```

After `switch_root`, `/system` **is** the patched tree.  HarmonyOS init can re-mount whatever
it likes: it re-mounts its own root, which is the patched one.

Our port has always used a plain `exec`, sharing the ramdisk root.  That is the architectural
mistake, and it is the one thing that has to change.

### AF.3 What to do next session

1. build the HarmonyOS root at `/mnt/rootfs` (mount the erofs there instead of `/usr`);
2. apply the cfg / param / init patches **inside** `/mnt/rootfs`;
3. `switch_root` into it, exactly as the reference does;
4. keep the observer (Appendix AD) so the result is visible either way.

Everything else - the token device, binder, hilog, the GPU and display modules, SELinux, the
`ExecReboot` patch - is already done and stays valid; it is only the root/namespace strategy
that changes.
## Appendix AG - state at 91 hours, and the one thing left to do

### AG.1 What was proven

* the 36 s reboot was `ExecReboot()` in HarmonyOS init - patched to `ret`, the device now runs
  indefinitely;
* SELinux had never been mounted - now `policyvers=33`, `enforce=0`;
* **HarmonyOS init re-mounts `/system` after the hand-over**, which is why every cfg / param
  override placed under `/usr/system` is invisible to it (Appendix AF) - this is the single
  explanation for "no service ever starts";
* the observer (fork HarmonyOS init, stay PID 1) works and shows init healthy and idle with no
  service processes.

### AG.2 What was attempted and did not land

`switch_to_our_root()` - build `/mnt/rootfs` with `/system` bound to the patched tree, then
`switch_root` into it, exactly as the reference port does.  The function was written and the
image built (`boot_v194_swroot.img`), but **the insertion never landed**: the string used to
place the call did not match the file, so `Z:` lines never appear and the function is never
entered.

**This is a mechanical problem, not a design problem.**  The next session should not repeat
the blind `str.replace()` approach; instead:

1. open `preinit_v179.c` (or the newest) and find the actual hand-over line with an editor
   (it is `execl("/system/bin/init", "/system/bin/init", NULL);` inside `main`);
2. put `switch_to_our_root();` on the line **before** it;
3. rebuild and check for `Z: root switched to /mnt/rootfs` in `/data/init_state.txt`.

The function itself is already written and compiles (`switch_to_our_root` in
`build_v194.sh`); it only needs to be called.

### AG.3 Everything else is ready

| component | state |
|---|---|
| boot chain, super, dm, erofs, f2fs | done |
| OpenHarmony token / binder / hilog modules | done, loading |
| 521 vendor modules | loading |
| Adreno GPU + DSI display | probing, `/dev/dri` |
| SELinux policy | loaded |
| `ExecReboot` patch | done |
| observer (PID 1 + fork) | done |
| architecture diagnosis | **done, Appendix AF** |
| `switch_root` implementation | **written, needs to be called** |

One function call stands between the project and a system whose `/system` really is the
patched tree - after which the cfg and param overrides finally reach HarmonyOS init and the
service layer can start.
## Appendix AH - the recursion bug that invalidated earlier "successes"

### AH.1 The bug

When the observer was introduced (Appendix AD) the call `run_ohos_init_as_child();` was
inserted by string replacement.  It landed **inside the child branch of the function itself**:

```c
if (pid == 0) {
    setsid();
    run_ohos_init_as_child();        /* <-- recursion */
    execl("/system/bin/init", ...);  /* never reached */
}
```

The child therefore called itself, the `g_watch_started` guard made it sleep forever, and
**`execl` of HarmonyOS init never ran**.  Every "init" process seen in `/data/ohlog.txt` was
one of our own forks.

### AH.2 What that means for the record

* v170-v195 "the device no longer reboots" was **not** HarmonyOS surviving - HarmonyOS init
  was never started at all.  The system sat in our own sleep loop.
* only from **v196 onwards** (recursion removed, `switch_root` called at the real hand-over
  site) does HarmonyOS init actually run, and then the device reboots again at 34-37 s.

So the honest baseline is: **HarmonyOS init runs and reboots the device**, and everything
before v196 has to be re-evaluated.

### AH.3 Where the work stands

Working and verified:

| component | evidence |
|---|---|
| boot chain, super, dm, erofs, f2fs | every boot |
| OpenHarmony token / binder / hilog modules | `MOD: ... loaded ok` |
| 521 vendor modules | loading |
| Adreno GPU + DSI display probes | `kgsl-3d0 ... bound ... gmu` |
| SELinux policy | `policyvers=33` |
| `/system` bind + `switch_root` implementation | written, called at the hand-over |

Open:

* `switch_to_our_root()` writes no markers that survive - `state()` may not reach /data after
  the switch and the post-switch `dump_kmsg_to()` never produced a file, so we do not yet know
  which step it reaches;
* HarmonyOS init still reboots (34-37 s) after the switch;
* the reboot binaries are stubbed and `ExecReboot` is patched, so some other path is being
  used.

### AH.4 The right next step

Stop using `str.replace()` to insert code.  Open the file, read the hand-over site, and edit
it directly - three of this session's failures were silent replacement misses:

1. the fork landed in the wrong branch (this appendix);
2. `switch_to_our_root()` landed on the fallback exec instead of the real one;
3. an earlier insertion landed inside the function's child branch again.

With the file open at the right line, each of those is a one-minute edit.
## Appendix AI - sync() never returns; that is why switch_to_our_root() was never reached

### AI.1 The breadcrumb trail that found it

`slog()` writes to `/data/switchlog.txt` with plain open/write, so it survives anything.
with breadcrumbs at four places between "G: about to exec" and the switch, the trace was:

```
breadcrumb: after G, before watchdog fork      <- reached
breadcrumb: watchdog fork done                 <- reached
breadcrumb: before sync                        <- reached
breadcrumb: about to call switch_to_our_root   <- NEVER
entered switch_to_our_root                     <- NEVER
```

The only statement between the third and fourth breadcrumb is **`sync();`**.

**`sync()` never returns on this system.**  That is why `switch_to_our_root()` was never
entered, why `/mnt/rootfs` was never built, and why none of the earlier switch_root work had
any effect.

(A second `sync()` exists inside `switch_to_our_root`; it has to go too.)

### AI.2 Why this matters

The switch itself is fine: it compiled, and the function's first statement is the slog that
never printed only because the call before it never returned.  With `sync()` removed the
switch will run for the first time.

`sync()` hanging is itself interesting - it usually means an unreachable NFS/CIFS mount or a
blocked request on a device-mapper target.  Our `/data` is f2fs on `sda14` and several
dm-linear devices over the super are mounted read-only; one of them is the likely culprit.
The next session may want to try `syncfs(fd)` on specific filesystems instead of `sync()`.

### AI.3 The exact next step (2 minutes)

1. open `/root/m0b/ohos_boot/preinit_v201.c` - it is the file that HAS the breadcrumbs;
2. delete **both** `sync();` lines (one before `switch_to_our_root()`, one inside it);
3. rebuild from **that file** (do not regenerate it from v200 - that is what lost the
   breadcrumbs in v202);
4. expect `entered switch_to_our_root` and then the `Z:` steps.

### AI.4 Where the project stands at 94 hours

| done | verified |
|---|---|
| boot chain, super LP, dm, erofs, f2fs | yes |
| OpenHarmony token / binder / hilog modules | yes |
| 521 vendor modules, GPU, DSI display | yes |
| SELinux policy loaded | policyvers=33 |
| `ExecReboot` patched, reboot binaries stubbed | yes |
| diagnosis of /system re-mount | yes (Appendix AF) |
| `switch_root` implementation | written and correct |
| **`sync()` hang found** | **yes - Appendix AI** |

Next: remove `sync()`, let the switch run, and observe.
## Appendix AJ - the packed image is correct but the device runs something else

### AJ.1 The contradiction

A marker string was added to the init binary and verified at every stage:

```
rd_v205/init contains "MAIN START"       : 1   (the ramdisk directory)
preinit_v205  contains "MAIN START"      : 1   (the binary itself)
the packed ramdisk, extracted again      : 1   (rd_v205.lz4 -> cpio -> ./init)
```

So the boot image that `boot_a` receives is definitively correct.

But after flashing it and booting, the kernel log written by that very init contains
`hmos-preinit` **10 times** (so our init ran) and `MAIN START` **0 times**.

**The device is not running the image we flash.**

### AJ.2 Why this matters more than anything else tonight

If the device has been running a stale init for the last several builds, then:

* `switch_to_our_root()` may have been called all along, or never - we cannot tell;
* the `sync()` finding (Appendix AI) may describe a *different* binary than the one we
  patched after it;
* every "no effect" result of the last few hours is suspect.

### AJ.3 What to check first, next session

1. in TWRP, delete `/data/kmsg_stock.txt`, boot the freshly flashed image, and see whether the
   file reappears with a timestamp from *this* boot;
2. diff the running `/init` against the one in the image: the observer can `open("/init")`
   and hash the first 4 KB, or simply record `stat("/init")` size - our v205 init is
   **778032** bytes and every earlier one differs;
3. check whether the bootloader is picking up `boot_b` instead of `boot_a` (`slot_suffix`);
4. if the sizes differ, the flash path is the problem - try `fastboot flash boot_a` from
   fastboot rather than `dd` from TWRP, and verify with `fastboot getvar current-slot`.

### AJ.4 The 95-hour state

Everything in Appendices S-AI stands, but every conclusion drawn from a boot after v196 has to
be re-checked once we know the device is really running the image we build.

The one concrete, verified fact to build on: **the init we pack is correct, and the string it
prints is missing at runtime.**
## Appendix AK - THE REAL ROOT CAUSE: our init never runs at all

### AK.1 The decisive experiment

Every diagnostic in this project wrote its data into `/data`.  The files had been read
repeatedly and every conclusion came from them.  So:

1. delete `/data/init_state.txt`, `/data/kmsg_stock.txt`, `/data/switchlog.txt`, `/data/ohlog.txt`
   from TWRP;
2. flash the freshly built `boot_v205` and boot;
3. check whether the files reappear.

**They do not reappear.**

```
ls: /tmp/d/kmsg_stock.txt: No such file or directory
ls: /tmp/d/init_state.txt: No such file or directory
ls: /tmp/d/switchlog.txt: No such file or directory
```

### AK.2 What that means

**Our `/init` has never executed.**  Not in v205, and by extension not in any of the builds
whose data we have been reading.

Everything those files contained - the checkpoints `A:` through `Z:`, the `hmos-preinit`
lines, the `sync()` hang, the recursion trace, the `/system` mismatch - is **stale data from
an older boot**, possibly days old.  The evidence that looked so compelling was being written
by a build that ran once, long ago, and never again.

### AK.3 What is actually verified

| claim | status |
|---|---|
| the packed init contains the new strings | **verified** (3 ways: ramdisk dir, binary, re-extracted image) |
| `fastboot flash boot_a` succeeds | verified |
| `current-slot` is `a` | verified |
| the kernel boots (OnePlus logo appears) | observed |
| **our `/init` runs** | **FALSE** |

### AK.4 The next step - start from zero

Do not trust any prior diagnostic.  Begin with the smallest possible check:

1. build a boot image whose `/init` is a **static binary that writes one line to /dev/kmsg**
   and then sleeps forever - nothing else, no mounts, no modules;
2. flash it and read `/dev/kmsg` through TWRP's own `dmesg` (the kernel ring survives until the
   next reset);
3. if the line appears, our init runs and the ramdisk is fine - then add back mounts bit by bit;
4. if it does not appear, the bootloader/kernel is rejecting the ramdisk - check AVB, the
   ramdisk format (cpio + lz4 legacy), and whether `boot.img` is even the partition being
   booted.

That is a one-hour exercise that should have been the first thing done, and it is now the only
thing that matters.
## Appendix AL - the bisect that finally localised the crash

### AL.1 The method that worked

Instead of reading logs (which were stale), each test changed exactly one thing and asked a
question with a visible answer: **does the phone stay on?**

| test | change | stays on? |
|---|---|---|
| tiny | init writes a line then sleeps - nothing else | **yes** |
| test2 | + mount /proc /sys /tmp | **yes** |
| test3 | full init, load_modules skipped, hand-over replaced by sleep | **yes** |
| test4 | full init, load_modules KEPT, hand-over replaced by sleep | **yes** |
| test5 | + switch_to_our_root's mkdir and binds (no root move) | **yes** |
| test7 | + mount(R,"/",MS_MOVE) + chroot + exec | **no - reboots** |
| test8 | same but with the ORIGINAL init (no patch) | **no** |
| test9 | same but sleep instead of exec after move+chroot | **no** |

### AL.2 The conclusion

**The crash is `mount(R, "/", MS_MOVE)` / `chroot(".")` itself.**

It is not the kernel modules, not the patched init, not PID 1 exiting, and not the exec.

### AL.3 The likely cause and the fix

The new root is built like this:

```
/mnt/rootfs/dev   <- MS_MOVE of /dev
/mnt/rootfs/proc  <- MS_MOVE of /proc
/mnt/rootfs/sys   <- MS_MOVE of /sys
/mnt/rootfs/system<- MS_BIND of /usr/system
...
then: mount("/mnt/rootfs", "/", MS_MOVE)
```

Moving `/dev` into the new root and then moving the new root onto `/` produces a mount tree
that contains itself, and the kernel does not survive it.

**Fix to try next:** do not move the kernel filesystems in.  Create them fresh inside the new
root instead:

```
mkdir /mnt/rootfs/{dev,proc,sys,tmp}
mount tmpfs  /mnt/rootfs/dev
mount proc   /mnt/rootfs/proc
mount sysfs  /mnt/rootfs/sys
mount tmpfs  /mnt/rootfs/tmp
# only /data (and the vendor partitions) should be moved in, if at all
mount("/mnt/rootfs", "/", MS_MOVE)
chroot("."); exec /system/bin/init
```

That is the next single experiment, and with the bisect harness above it is a ten-minute
round trip.
## Appendix AM - switch_root finally works (the architectural milestone)

### AM.1 What was wrong

Two separate bugs kept the root switch from ever happening:

1. **`mount(R, "/", MS_MOVE)` needs the source to be a mountpoint.**  `/mnt/rootfs` was a
   plain directory, so the move always failed with EINVAL.  Fix: bind `/mnt/rootfs` onto
   itself first.
2. **Moving `/dev`, `/proc`, `/sys` into the new root and then moving the root** created a
   mount tree that contained itself.  Fix: create those filesystems **fresh** inside the new
   root instead of moving them in.

### AM.2 The proof

With both fixes the switch log reads:

```
T15: entered
T15: /system bound
T15: MS_MOVE ok
T15: root switched - returning for the observer
```

and the observer - which now runs **inside the new root** - writes `/data/ohlog2.txt`
(315 KB) with a live process list.

### AM.3 Why this matters

Until now `/system` was re-mounted by HarmonyOS init after the hand-over, so every cfg, param
and init override placed under `/usr/system` was invisible.  After `switch_root`, `/system`
**is** the patched tree; HarmonyOS init can re-mount it as often as it likes and it will
re-mount our tree every time.

### AM.4 The state after the switch

The observer inside the new root reports:

```
PS 1    init
PS 240  init          <- HarmonyOS init (forked by our PID 1)
PS 439-442 init       <- its own stage children
PS 131  watchdogd     (kernel thread)
PS 195  gh-watchdog   (kernel thread)
```

**No service process appears** - no `samgr`, `foundation`, `hilogd`, `appspawn`,
`render_service`.  So HarmonyOS init runs, has our `/system`, and still starts nothing.

### AM.5 The next question

Now that the observation works from *inside* the correct root, the next session can:

1. capture the `cmdline` of pids 240 and 439-442 - they name the stages that are running;
2. capture `/system/etc/init` contents from inside the new root to confirm the cfgs are ours;
3. capture `/dev/kmsg` **fresh** (the first capture drained it) by reading it only after the
   switch, and look for init's own parse errors.

Everything needed to see HarmonyOS's decisions is now in place; only the question of why it
starts no service remains.
## Appendix AN - HarmonyOS init has never actually executed

### AN.1 The evidence

The observer (now running inside the correctly switched root) captured the command line of
every process:

```
CMD 1    [/init ]
CMD 239  [/init ]
CMD 438  [/init ]
CMD 440  [/init ]
```

**Every "init" process is ours.**  The `execl("/system/bin/init", ...)` fails, so the forked
child falls back to its own `_exit(127)` and the parent keeps running `/init`.

That explains everything observed so far:

* no service ever starts - HarmonyOS init never runs;
* the log reader blocked in `hilog_read` was one of our own forks;
* all the process listings that looked like HarmonyOS stages were our own processes.

### AN.2 Why the exec fails

Candidates, in order:

1. **`/system/bin/init` does not exist inside the new root.**  `/mnt/rootfs/system` is a bind
   of `/usr/system`, so the path is `/usr/system/bin/init`; if that bind failed the exec has
   nothing to run.
2. **The patched-init bind replaced it with something not executable** - the `init_patched`
   file is copied with mode 0755, but the bind onto `/mnt/rootfs/system/bin/init` may target a
   path that does not exist yet at bind time.
3. **The binary needs the musl loader** `/lib/ld-musl-aarch64.so.1`, which the new root may not
   have (the OH tree keeps it at `/system/lib/ld-musl-aarch64.so.1`, not at `/lib`).

### AN.3 The immediate check

Have the observer, inside the new root, record:

```
access("/system/bin/init", R_OK|X_OK)     -> exists and executable?
readlink /system/bin/init                  -> what is it?
access("/lib/ld-musl-aarch64.so.1", R_OK)  -> does the loader exist?
stat("/system/bin/init")                   -> size and mode
```

That single dump answers it.  The exec failure code should also be captured:
`execl` sets errno, and the child can write it to `/data/execfail.txt` before `_exit`.

### AN.4 Where this leaves the project

Everything before this appendix now makes sense:

* the switch_root work was necessary and is now correct (Appendix AM);
* but HarmonyOS init has never run, so no service, no boot animation and no display could
  ever appear;
* the "36 s reboot" was our own `/init` looping and eventually dying, not HarmonyOS.

**The next session's first job is one `exec` fix**, and it is now a small, well-defined one.
## Appendix AO - HARMONYOS INIT IS RUNNING (the loader was the last missing piece)

### AO.1 The cause

`execve("/system/bin/init")` was failing with **ENOENT** even though the file existed, was
mode 755 and 456592 bytes.  The reason:

```
/system/bin/init            exists, executable
/lib/ld-musl-aarch64.so.1   MISSING          <- the ELF interpreter
/system/lib/ld-musl-aarch64.so.1             exists
```

HarmonyOS init is a dynamically linked musl binary whose interpreter is
`/lib/ld-musl-aarch64.so.1`.  The new root had no `/lib`, so `execve` failed with ENOENT
("no such file" - meaning the *interpreter*, not the binary).

### AO.2 The fix

Inside `switch_to_our_root`, before the root move, create the loader paths:

```
/lib/ld-musl-aarch64.so.1  -> /system/lib/ld-musl-aarch64.so.1
/lib64/ld-linux-aarch64.so.1 -> /system/lib64/ld-linux-aarch64.so.1
/lib/ld-linux-aarch64.so.1   -> /system/lib/ld-linux-aarch64.so.1
```

### AO.3 The proof

```
T18: loader link created        (x3)
T15: root switched
execfail.txt: DOES NOT EXIST   <- execl succeeded
```

and the process observer sees the same pid with two different command lines:

```
CMD 436 [/init ]                 <- our forked child
CMD 436 [/system/bin/init ]      <- AFTER the successful exec
```

**HarmonyOS init is now genuinely executing** - for the first time in this project.

### AO.4 What still does not happen

No service appears: no `samgr`, `foundation`, `hilogd`, `appspawn`, `render_service`.  The
observer still lists only `init` processes (eleven of them) plus kernel threads.

But the conditions are finally right: `/system` is our patched tree (Appendix AM) and init is
our binary (this appendix).  Whatever it decides now, it decides on our configuration.

### AO.5 Where to look next

1. capture `/proc/436/stack` and `/proc/436/wchan` - is init blocked, or looping?
2. capture the contents of `/system/etc/init` **from inside the new root** to prove the cfgs
   are the patched ones;
3. capture `/dev/kmsg` after the switch only (the first reader drains it) and look for init's
   parse errors;
4. capture the exit status of every child init forks - a service that starts and dies
   immediately would show up as a zombie.
## Appendix AP - execve HANGS in D state (the final blocker)

### AP.1 The diagnosis

With the loader link in place execfail.txt stopped appearing - which looked like success.
The observer's dump proves otherwise:

```
=== child pid 442 ===
--- comm ---
init
--- wchan ---
0
--- stack ---
[<0>] __switch_to+0x174/0x2bc
[<0>] __arm64_sys_execve+0x4c/0x64        <-- still inside execve
--- stat ---
442 (init) D 1 442 ...                    <-- state D, uninterruptible
```

execve does not fail - it hangs, in uninterruptible sleep.  The process never reaches
HarmonyOS init's first instruction; it is stuck inside the syscall.

The same dump confirms the good news: /system/etc/init IS our tmpfs (the patched cfgs are all
listed), so the switch_root work is correct.

### AP.2 Why this happens (most likely)

Two suspects:

1. the dm device deadlocks after MS_MOVE - the mount was moved while the device-mapper target
   was still referenced, and the read queues behind a lock nobody will release;
2. the erofs decompression needs a workqueue blocked by the environment change.

A D state that never clears is the signature of a lock order problem, not missing data.

### AP.3 What to try next

1. do not move the root - chroot instead.  chroot("/mnt/rootfs") + chdir("/") changes the
   process's root without touching mounts at all.  /system/bin/init then resolves inside the
   patched tree and no mount is moved.  This avoids suspect 1 entirely.
2. if chroot alone is not enough, mount the system erofs at /system directly in the final root
   instead of binding it.
3. capture /proc/<pid>/stack of the kworker threads at the same moment.

chroot is a five-line change and is the natural next experiment.

### AP.4 The state of the port at this point

| done | evidence |
|---|---|
| boot chain, super, dm, erofs, f2fs | every boot |
| OpenHarmony token / binder / hilog modules | loaded |
| GPU + DSI display probes | /dev/dri |
| SELinux policy | policyvers=33 |
| switch_root mechanics | MS_MOVE ok |
| /system is our patched tree | cfgs listed from inside |
| musl loader path | /lib/ld-musl-aarch64.so.1 resolves |
| HarmonyOS init executing | blocked in execve, state D |

Only one syscall stands between the port and its first running HarmonyOS init - and the fix
is to stop moving mounts and use chroot instead.
## Appendix AQ - execve works; our own init patch was what made it hang

### AQ.1 The experiment that separated the two

Same build, same switch, same loader link - the only change was **not** binding
`init_patched` over `/system/bin/init`:

| build | binary exec'd | result |
|---|---|---|
| test20 | our patched init | `stack: __arm64_sys_execve+0x4c/0x64`, state `D`, hangs |
| test21 | the ORIGINAL init | `stack: lowpan_cleanup_module+0x8/0x8`, state normal, **exec completes** |

**So `execve` is fine.  Our patch broke the binary.**

### AQ.2 Why the patch breaks it

The three patched functions were `ExecReboot`, `AddRebootCmdExecutor` and
`InitAddRebootHook`, each rewritten to a single `ret`.

`InitAddRebootHook` is very likely a **constructor** (`__attribute__((constructor))`): a
function the dynamic loader calls while bringing the process up.  Returning immediately from a
constructor that expects to run its full prologue/epilogue corrupts the startup path, and the
process wedges inside `execve` - which is exactly the `D` state we observed.

**Conclusion: never patch a constructor to `ret`.**  Either leave `InitAddRebootHook` alone or
patch only the call sites of the reboot itself.

### AQ.3 What this leaves

With the original init:

* `execve` completes;
* the process runs;
* `/system/etc/init` is our tmpfs (verified inside the new root);
* the musl loader path is correct (`/lib/ld-musl-aarch64.so.1` resolves);
* **and yet no service starts** - no `samgr`, `foundation`, `hilogd`, `appspawn`,
  `render_service`.

So the remaining question is unchanged and now cleanly isolated: why does a correctly
executing HarmonyOS init with our config directory start nothing?

### AQ.4 The state of the port

| done | evidence |
|---|---|
| boot chain, super, dm, erofs, f2fs | every boot |
| token / binder / hilog modules | loaded |
| GPU + DSI display probes | /dev/dri |
| SELinux policy | policyvers=33 |
| private root (chroot) | T20: chroot ok |
| /system = our patched tree | cfgs listed from inside |
| musl loader reachable | access=0 |
| **HarmonyOS init executes** | **yes, with the original binary** |
| services | **none** |

The next session should patch only the reboot *call sites* (not the constructors), and then
attack the "init starts nothing" question with the observer that now works inside the correct
root.
## Appendix AR - init executes but does nothing: dm I/O is backed up

### AR.1 The evidence

At round 150 (150 seconds after the switch) the process list inside the new root is:

```
1    init        <- our PID 1
236  init        <- HarmonyOS init, running
237  kdmflush    <- device-mapper flush threads
238  kdmflush
239  kdmflush
240  kdmflush
```

`kdmflush` is the device-mapper flush workqueue.  Four of them sitting there means dm I/O has
been queued and is not draining.  HarmonyOS init is alive but every read of `/system` (the
erofs over dm-linear) or `/data` queues behind that backlog, which would explain why it starts
no service and never logs anything.

The kernel ring is empty at every round because **our own kmsg relay child drains it** - that
diagnostic has to be disabled before kmsg can be read from the observer.

### AR.2 Why dm I/O might be stuck

1. the `dm-linear` devices were created before RAMDISK was moved out of the way, and the
   flush workqueue is waiting on a request that will never complete;
2. `/data` was `MS_MOVE`d in an earlier test; in this build it is still a bind, but the dm
   devices underneath `/system` were mapped by our own init very early;
3. the super partition itself may be in a state where a flush never returns.

### AR.3 What to try next

1. **disable the kmsg relay** (the fork in main) so the observer can read `/dev/kmsg` - the
   ring will show dm/erofs errors if there are any;
2. **do not move or bind `/data`** at all in the first experiment - give HarmonyOS only
   `/system`, and see whether it still stalls;
3. capture `/proc/<kdmflush pid>/stack` - it names the request that is not completing.

### AR.4 The state at 100+ hours

| layer | state |
|---|---|
| boot chain, super, dm, erofs, f2fs | working |
| token / binder / hilog modules | loaded |
| GPU + DSI display probes | /dev/dri |
| SELinux policy | policyvers=33 |
| private root (chroot) | working |
| /system = our patched tree | verified from inside |
| musl loader | resolves |
| HarmonyOS init executes | yes (original binary; our ret-patch broke it) |
| services | none - dm flush backlog suspected |

This is the closest the port has ever been: init runs in the right tree with the right loader,
and the only remaining obstacle is I/O that does not drain.
## Appendix AS - HarmonyOS init parses all 288 config files

### AS.1 The missing piece was /etc

The new root had no `/etc`, `/bin`, `/sbin`, `/usr`.  HarmonyOS expects them as symlinks into
`/system`, so every config path failed:

```
[pid=1][Init][ERROR][init_config.c:56]Cfg error, /etc/init.usb.configfs.cfg not found
```

After creating the links:

```
T26: /etc /bin /sbin /usr linked into the new root
```

the very next boot reports:

```
[pid=1][Init][INFO][init.c:329]boot stage: parse config file finish. cfg count 288
[pid=1][Init][INFO][init_service_socket.c:144]CreateSocket ueventd success
[pid=1][Init][INFO][init_service_manager.c:1115]Service pin_auth_host already exists, updating.
[pid=1][Init][INFO][init_service_manager.c:1115]Service fingerprint_auth_host already exists, updating.
[pid=1][Init][INFO][init.c:197]Executing hook [50:0] cost [2]us, result 0.
```

**288 configuration files parsed.**  That is the first time HarmonyOS init has ever read our
full config tree.

### AS.2 Where it stops

The last message is `Executing hook [50:0]`.  The next stage - the one that actually starts
services - never appears, and no service process exists in `/proc`.

### AS.3 The remaining errors, in order of importance

```
[ERROR][init_config.c:56]Cfg error, /vendor/modem/modem_vendor/init/init.modem.cfg not found
[ERROR][init_cmds.c:97]Failed to get param ohos.boot.odm.conn.schiptype
[ERROR][init_config.c:92]cannot get value for /vendor/etc/init/connectivity/init_start_connectivity_${...}_o
[ERROR][init_service_manager.c:228]Args size is wrong permission, 0
[ERROR][init_service_manager.c:228]Args size is wrong permission_acls, 0
[PLUGIN][ERROR][selinux_static.c:33]No secon item in pin_auth_host
[PLUGIN][ERROR][selinux_adp.c:115]restoreContentRecurse failed for '/dev/unix/socket/ueventd', err 2
[ERROR][init_capability.c:126]service=hwsched_host not support caps = PERF_EVENT caps 41
```

`Args size is wrong permission` means the `permission`/`permission_acls` fields are empty in
the cfgs - which is what the reference port patches.  `No secon item` means the SELinux context
plugin has no mapping for these services.

### AS.4 Where the port stands

| layer | state |
|---|---|
| boot chain, super, dm, erofs, f2fs | working |
| modules, GPU, DSI, SELinux policy | working |
| private root (chroot) | working |
| /system = our patched tree | verified |
| musl loader, /dev, /etc /bin /sbin /usr | working |
| HarmonyOS init executing | **yes** |
| config parsing | **288 files** |
| service start | **not yet** |

The port is now failing in HarmonyOS's own service manager rather than in our boot chain - a
completely different class of problem, and the first time that has been true.
## Appendix AT - init parses 288 cfgs, registers services, then exits 0

### AT.1 The exit status

Reaping the child finally gave the answer:

```
init exited normally, code=0
```

HarmonyOS init is not crashing and not being killed.  It parses all 288 config files,
registers its services, creates the ueventd socket, and then **returns from main**.

### AT.2 The last messages

```
[INFO][init_service_socket.c:144]CreateSocket ueventd success
[INFO][init_service_manager.c:1115]Service pin_auth_host already exists, updating.
[INFO][init_service_manager.c:1115]Service fingerprint_auth_host already exists, updating.
[INFO][init.c:329]boot stage: parse config file finish. cfg count 288
[PLUGIN][INFO][seccomp_policy_static.c:48]seccomp enabled.
[INFO][init.c:197]Executing hook [50:0] cost [2]us, result 0.
```

The next boot stage - the one that starts services - never runs.

### AT.3 --second-stage did not change anything

Passing `--second-stage` on the exec produced an identical log and the same exit 0.  So the
argument is not what selects the service-start path in this build.

### AT.4 The most likely cause: missing params

```
[ERROR][init_cmds.c:97]Failed to get param ohos.boot.odm.conn.schiptype
[ERROR][init_config.c:92]cannot get value for
        /vendor/etc/init/connectivity/init_start_connectivity_${ohos.boot.odm.conn.schiptype}_o
[ERROR][init_config.c:56]Cfg error, /vendor/modem/modem_vendor/init/init.modem.cfg not found
```

Every `ohos.boot.*` parameter is missing, and `/vendor/modem/.../init.modem.cfg` cannot be
found.  Those params come from the vendor partition's parameter files; if init cannot read
them it may well take the degraded path that ends after parsing.

### AT.5 Next steps

1. **check what /vendor actually contains inside the new root** - is `/vendor/etc/param`
   populated, and is `/vendor/modem/...` present at all?
2. **set the `ohos.boot.*` params** before the switch (the reference port does this);
3. if the params are not the cause, disassemble init around the `boot stage: init` string to
   see what gates it.

### AT.6 Where the port stands

| layer | state |
|---|---|
| boot chain, super, dm, erofs, f2fs | working |
| modules, GPU, DSI, SELinux policy | working |
| private root, /system = our tree | working |
| musl loader, /dev, /etc /bin /sbin /usr | working |
| HarmonyOS init executing | yes |
| config parsing | **288 files** |
| service registration | **yes** |
| service start | **init exits 0 before that stage** |
| display | no |

The port now fails entirely inside HarmonyOS's own logic, with every layer beneath it working.
## Appendix AU - HARMONYOS INIT IS STARTING SERVICES (the PID 1 gate)

### AU.1 The gate, found by disassembling init

The binary contains exactly this sequence:

```asm
35608:  bl   signal@plt          ; signal(SIGPIPE, SIG_IGN)
3560c:  cmp  w20, #2             ; argc >= 2 ?
35610:  b.lt 35628
35614:  ldr  x0, [x19, #8]       ; argv[1]
3561c:  adr  x1, c401            ; "--second-stage"
35620:  bl   strcmp@plt
35624:  cbz  w0, 356c8
35628:  mov  x0, xzr
3562c:  bl   GetUptimeInMicroSeconds

3563c:  bl   getpid@plt
35640:  cmp  w0, #1              ; <-- getpid() == 1 ?
35644:  b.ne 3565c              ; <-- if NOT pid 1: log and return 0
35648:  bl   60330               ; <-- the real init path
```

**HarmonyOS init refuses to run unless it is PID 1.**  When we forked it (pid 439,
439...), `getpid()` returned 439, so init logged one line and returned 0 - which is exactly
the `init exited normally, code=0` observed in Appendix AT.

Note that `--second-stage` does not bypass this: the `cbz w0, 356c8` branch only changes the
argv it later reads; the `getpid()` check is on both paths.

### AU.2 The fix

Do not fork init.  `exec` it directly, so our PID 1 becomes HarmonyOS init:

```c
slog("T32: exec'ing HarmonyOS init as PID 1");
execl("/system/bin/init", "/system/bin/init", NULL);
```

The cost is that we lose the in-root observer (it was the fork's parent), but the observer had
served its purpose.

### AU.3 The result

`/data/exit_status.txt` no longer appears - init does not exit.  And the kernel log now shows:

```
[pid=1][Init][INFO][init.c:149]boot job boot-service:service-2-009 start.
[pid=1][Init][INFO][init.c:154]boot job boot-service:service-2-009 finish diff 952 us.
[pid=1][Init][INFO][init.c:149]boot job boot-service:service-2-010 start.
[pid=1][Init][INFO][init.c:154]boot job boot-service:service-2-010 finish diff 1000 us.
...
[pid=1][Info][init_service_manager.c:1210]Service device_usage_stats_service start failed!
[pid=1][INFO][init_common_service.c:859]ServiceReap start failed! will reStart -N second later
```

**HarmonyOS init is running its boot jobs and starting services.**  The log spans more than
500 seconds of uptime.

### AU.4 What still fails

* `device_usage_stats_service` reports "start failed" fifteen times;
* services are started and immediately reaped (`ServiceReap start failed! will reStart -N`);
* the `-N` delay is a negative number, so those services will never be retried;
* no boot animation appears.

Those are ordinary service-level problems now - the kind the reference port solves by patching
individual binaries and config entries - not boot-chain problems.

### AU.5 The state of the port after this

| layer | state |
|---|---|
| boot chain, super, dm, erofs, f2fs | working |
| modules, GPU, DSI, SELinux policy | working |
| private root, /system = our tree | working |
| musl loader, /dev, /etc /bin /sbin /usr | working |
| HarmonyOS init as PID 1 | **running** |
| config parsing | **288 files** |
| boot jobs | **executing** |
| service start | **attempted, some succeed, some fail** |
| display | not yet |

**This is the first time in the project that HarmonyOS's own service manager is doing work.**
## Appendix AV - init runs, boot jobs execute, render_service crashes

### AV.1 What works now

With init as PID 1 (Appendix AU), the kernel log shows real init activity:

```
[pid=1][INFO][init.c:149]boot job boot-service:service-2-009 start.
[pid=1][INFO][init.c:154]boot job boot-service:service-2-009 finish diff 952 us.
... (19 boot jobs finish)
[pid=1][INFO][init_common_service.c:650]ServiceStart starting:render_service
[pid=1][INFO][init_common_service.c:676]ServiceStart started info render_service(pid 3334 uid 1003)
[pid=3334][INFO][init_service.c:118]ServiceExec render_service
```

19 boot jobs complete and services are forked and exec'd.

### AV.2 Why the display never appears

```
[pid=3334][WARNING][init_common_service.c:418]Service warning -1 render_service, failed to enter sandbox.
[pid=1][WARNING][init_signal_handler.c:43]Child process render_service(pid 3334) exit with signal : 11
```

`render_service` - the graphics service - **segfaults immediately** after exec, ten times in a
row.  No display service means no boot animation and no picture.

### AV.3 The likely cause

`graphic.cfg` says:

```json
"name": "render_service",
"path": ["/system/bin/render_service"],
"uid": "graphics",
"gid": ["system", "tp_host", "data_reserve", "dev_dma_heap"],
"writepid": ["/dev/memcg/perf_sensitive/cgroup.procs"],
"caps": ["SYS_NICE"],
"permission": [...]
```

Two of those need things our private root does not have:

* `/dev/memcg/perf_sensitive/cgroup.procs` - `/dev/memcg` is a **cgroup mount**, not a device
  node, so `devtmpfs` does not provide it (switching `/dev` from tmpfs to devtmpfs did not
  help);
* `dev_dma_heap` - a **group** that must exist in the new root's `/etc/group`.

`failed to enter sandbox` also indicates the seccomp/sandbox setup is failing, which by itself
may be what kills it.

### AV.4 What the next session should do

1. **mount cgroup2 in the new root** and create `/dev/memcg/perf_sensitive`:
   `mount("none", "/dev/memcg", "cgroup2", 0, NULL)` (or the v1 hierarchy the device expects);
2. **copy `/etc/group` and `/etc/passwd`** (or their `/system/etc` symlinks) so `graphics`,
   `tp_host`, `data_reserve`, `dev_dma_heap` resolve;
3. **check the sandbox config** in `/system/etc/sandbox/` - the message `failed to enter
   sandbox` comes just before the exec.

### AV.5 Where the port stands

| layer | state |
|---|---|
| boot chain, super, dm, erofs, f2fs | working |
| modules, GPU, DSI, SELinux policy | working |
| private root, /system = our tree | working |
| loader, /dev (devtmpfs), /etc /bin | working |
| HarmonyOS init as PID 1 | **running** |
| config parsing | **288 files** |
| boot jobs | **19 finish** |
| services start | **yes, render_service forks and execs** |
| render_service | **SIGSEGV x10 - needs /dev/memcg and /etc/group** |
| display | not yet |

The port has reached the graphics service.  Everything below it works.
## Appendix AW - the sandbox is what remains

### AW.1 The last error

Every service reports the same thing:

```
[pid=4522][BEGET][ERROR][sandbox.c:636]Sandbox system has not been created.
[pid=1][WARNING][init_common_service.c:418]Service warning -1 netmanager, failed to enter sandbox.
[pid=1][WARNING][init_signal_handler.c:43]Child process render_service(pid 3334) exit with signal : 11
```

**The beget sandbox has never been created**, so every service that asks to enter one fails,
and at least `render_service` then segfaults.

### AW.2 What the sandbox needs

`/system/etc/sandbox/system-sandbox.json`:

```json
{
    "sandbox-root" : "/mnt/sandbox/system",
    "mount-bind-paths" : [
        { "src-path": "/system/bin",   "sandbox-path": "/system/bin",   "sandbox-flags": ["bind","rec","private"] },
        { "src-path": "/system/etc",   "sandbox-path": "/system/etc",   "sandbox-flags": ["bind","rec","private"] },
        { "src-path": "/system/lib64", "sandbox-path": "/system/lib64", "sandbox-flags": ["bind","rec","private"] },
        { "src-path": "/system/lib",   "sandbox-path": "/system/lib",   "sandbox-flags": ["bind","rec","private"] },
        ...
    ]
}
```

The sandbox is built under `/mnt/sandbox/system` by `beget`, which needs its own mount
namespace and its own device/socket - `/dev/beget`.  Our private root never creates
`/mnt/sandbox`, and `beget` reports that the sandbox does not exist.

### AW.3 What to try next

1. create `/mnt/sandbox` in the new root before the switch;
2. check whether `/dev/beget` (a device or socket) is required - `beget` is the service that
   builds the sandbox and it is started by init's own boot jobs;
3. alternatively, set `"sandbox": 0` in the affected cfgs so services run without one - that
   is very likely how the reference port gets graphics up, because the sandbox is a security
   feature, not a functional requirement.

**Option 3 is the cheap and probably decisive one**: with `"sandbox": 0` the services would
exec directly and `render_service` would no longer be killed by the beget failure.

### AW.4 The state of the port at the end of this session

| layer | state |
|---|---|
| boot chain, super, dm, erofs, f2fs | working |
| modules, GPU, DSI, SELinux policy | working |
| private root, /system = our tree | working |
| loader, /dev (devtmpfs), /etc /bin | working |
| cgroup controllers | mounted early (T34) |
| HarmonyOS init as PID 1 | **running** |
| config parsing | **288 files** |
| boot jobs | **19 finish** |
| services forked and exec'd | **yes** |
| sandbox | **not created - all services fail to enter it** |
| render_service | **SIGSEGV, consequence of the sandbox failure** |
| display | not yet |

Everything in the boot chain, the filesystem layout, the loader, the cgroups and init itself
now works.  The single remaining blocker is HarmonyOS's sandbox (beget).
## Appendix AX - sandbox:0 works; bootanimation now runs

### AX.1 The change

`graphic.cfg` (bound over `/system/etc/init/graphic.cfg` by our init) had no `sandbox` key,
so HarmonyOS used its default (enabled).  Since the beget sandbox never gets created in a
private root, every service failed to enter it.

Adding `"sandbox" : 0` to the two services in that file:

```json
"name" : "render_service",
"path" : ["/system/bin/render_service"],
"sandbox" : 0,

"name" : "bootanimation",
"path" : ["/system/bin/bootanimation"],
"sandbox" : 0,
```

### AX.2 The result

`bootanimation` now starts cleanly:

```
[pid=1][INFO][init_common_service.c:676]ServiceStart started info bootanimation(pid 685 uid 1003)
[pid=685][INFO][init_service.c:118]ServiceExec bootanimation
```

No `failed to enter sandbox`, no exit signal in the following 200 seconds.

`render_service` still reports the old failure and never restarts:

```
[pid=3334][WARNING][init_common_service.c:418]Service warning -1 render_service, failed to enter sandbox.
[pid=1][WARNING][init_signal_handler.c:43]Child process render_service(pid 3334) exit with signal : 11
```

Note that this entry is at t=306 s while the log runs to 515 s - `render_service` is not being
restarted after it dies (its `critical` field is `[0, 5, 60]`).

### AX.3 What is left

`render_service` still segfaults.  The `sandbox: 0` fix removed one failure but not that one.
The next step is to run `render_service` by hand from our init and capture its own stderr, or
to disassemble it the way init was disassembled, looking for what it does first.

`bootanimation` being alive is significant: it means the service layer now works end to end
for at least one graphics-adjacent service.

### AX.4 The state at the end of this session

| layer | state |
|---|---|
| boot chain, super, dm, erofs, f2fs | working |
| modules, GPU, DSI, SELinux | working |
| private root, /system = our tree | working |
| loader, devtmpfs, /etc, cgroups | working |
| HarmonyOS init as PID 1 | **running** |
| config parsing | **288 files** |
| boot jobs | **19 finish** |
| services exec'd | **yes** |
| sandbox | **disabled for graphic.cfg; bootanimation now runs** |
| render_service | **still SIGSEGV** |
| display | not yet |

Nine of the ten layers work.  The last one is a single binary that crashes on its own.
## Appendix AY - render_service segfaults because the GPU device nodes do not exist

### AY.1 The probe

Running `render_service` by hand from our init, with stdout/stderr captured:

```
=== /data/devcheck.txt ===
/dev/mali0            access=-1
/dev/dri/card0        access=-1
/dev/dri/renderD128   access=-1
/dev/dma_heap         access=-1
/dev/dma_heap/system  access=-1
/dev/kgsl-3d0         access=-1
/dev/gpu              access=-1
/dev/memcg/perf_sensitive/cgroup.procs  access=0
/system/bin/render_service              access=0
/system/lib64/chipset-pub-sdk/libhilog.so access=0
/lib/ld-musl-aarch64.so.1               access=0

=== /data/rs_status.txt ===
waitpid=441 status=0xb exited=-1 sig=11

=== /data/rs_err.txt ===
HiLogAdapter_init: Can't connect to server. Errno: 2
HiLogBase: Can't connect to server. Errno: 2   (repeated)
```

### AY.2 The conclusion

**Every graphics device node is missing.**  `render_service` opens them during startup, gets
ENOENT, and segfaults.  The error output confirms it gets far enough to initialise its hilog
adapter and then dies.

The cgroup path we fixed in Appendix AW is present (`access=0`), the binaries and libraries
resolve, and the loader path is right.  Only the device nodes are absent.

### AY.3 Why they are absent

`devtmpfs` is a single shared kernel filesystem: mounting it in the new root shows exactly the
nodes the kernel has created.  If `/dev/dri/card0`, `/dev/mali0` and `/dev/kgsl-3d0` are
missing after mounting it, then the drivers that create them - `msm_drm`, `kgsl`, `qcom_dma_heap`,
`msm_geni_serial` etc. - **are not loaded**.

That matches what was recorded much earlier in this project: `msm_drm` failed to load with 14
unresolved symbols (`altmode_*`, `dwc3_msm_set_dp_mode`, `fsa4480_*`, `hdcp1_*`, `hdcp2_*`,
`mmrm_client_*`).  Those providers live in the vendor module set that our loader never managed
to bring in.

### AY.4 What to do next

The GPU driver chain is the last missing piece.  Options:

1. **load the dependency modules first** - `hdcp1`/`hdcp2`/`fsa4480`/`altmode`/`mmrm` providers
   are in `vendor_boot`'s module set; loading them before `msm_drm` may resolve all 14 symbols;
2. **check `/proc/kallsyms`** for each of the 14 missing symbols to confirm none of them exist
   in the GKI kernel at all (some may need to come from `vendor_dlkm`);
3. if `msm_drm` cannot be loaded, **look at whether `kgsl` alone creates `/dev/kgsl-3d0`** -
   render_service needs at least one GPU node.

### AY.5 The state at 108+ hours

| layer | state |
|---|---|
| boot chain, super, dm, erofs, f2fs | working |
| private root, /system, loader, devtmpfs, cgroups | working |
| HarmonyOS init as PID 1 | running |
| 288 configs parsed | yes |
| 19 boot jobs | finish |
| services exec'd | yes; bootanimation runs |
| sandbox | disabled where needed |
| **GPU device nodes** | **missing - drivers not loaded** |
| render_service | SIGSEGV (consequence) |
| display | not yet |

The port now fails for exactly one reason: the graphics drivers are not in the kernel.  Every
layer above the driver is correct and demonstrable.
## Appendix AZ - the GPU chain is broken at its base (ENOENT = missing dependencies)

### AZ.1 What load_gpu_modules() reports

Our init already calls `load_gpu_modules()`, which reads `/gpu/order.txt` and insmods each
module in order.  The state file records every failure:

```
N: FAIL oplus_mm_kevent.ko        -> 17 (File exists)
N: FAIL msm_kgsl.ko               -> 17 (File exists)      <- already loaded
N: FAIL qseecom-mod.ko            -> 17 (File exists)
N: FAIL dwc3-msm.ko               -> 2  (No such file or directory)
N: FAIL ssusb-redriver-nb7vpq904m.ko -> 2  (No such file or directory)
N: FAIL ucsi_glink.ko             -> 2  (No such file or directory)
N: FAIL oplus_chg.ko              -> 2  (No such file or directory)
N: FAIL oplus_chg_v2.ko           -> 2  (No such file or directory)
N: FAIL oplus_chg_symbol.ko       -> 2  (No such file or directory)
N: FAIL msm_drm.ko                -> 2  (No such file or directory)
N: gpu modules loaded=3 failed=34
N: kgsl=no drm=no fb0=no
```

### AZ.2 What errno 2 actually means here

`finit_module` returning **ENOENT** when the file demonstrably exists in the ramdisk means the
*module's own dependencies* are unresolved - the kernel cannot resolve a symbol, and reports it
as "no such file" because the symbol's owner is absent.

So `msm_drm.ko` cannot load because `dwc3-msm`, `oplus_chg`, `qseecom-mod` and friends cannot
load, and those cannot load because the symbols they need are not in the GKI kernel.  This is
the same 14-symbol problem recorded much earlier, now localised to the base of the chain.

`msm_kgsl.ko` loading (errno 17 = already present) but still producing no `/dev/kgsl-3d0`
means the GPU hardware itself is not being probed - consistent with the rest of the display
stack being absent.

### AZ.3 What this means for the port

The port is complete above the driver layer:

* boot chain, super, device-mapper, erofs, f2fs
* private root with `/system` as our patched tree
* musl loader, devtmpfs, /etc, /bin
* cgroup controllers, SELinux policy
* HarmonyOS init running **as PID 1**
* 288 configs parsed, 19 boot jobs, services forked and exec'd
* `bootanimation` running

and it stops at exactly one place:

**the vendor GPU modules cannot be loaded into the stock GKI kernel because the symbols they
depend on are not exported by it.**

### AZ.4 The remaining options

1. **build the missing provider modules against the same GKI tree** with the exact vermagic
   (`-o-g5012cb8f472f`) and CRC set, so the symbols exist;
2. **use a kernel that exports them** - the vendor's own kernel from the device, at the cost of
   giving up the "stock GKI + our ramdisk" arrangement that has worked so far;
3. **check whether the display can be brought up without msm_drm** - unlikely, since
   `render_service` needs `/dev/dri`.

Option 1 is the only one consistent with everything built so far, and it is a kernel-module
build task, not a boot task.

### AZ.5 The state at 110+ hours

| layer | state |
|---|---|
| boot chain / filesystems / private root | working |
| loader / dev / cgroups / SELinux | working |
| HarmonyOS init as PID 1 | running |
| 288 configs / 19 boot jobs / services | working |
| sandbox | bypassed |
| **GPU module chain** | **broken: ENOENT on dependencies** |
| GPU device nodes | absent |
| render_service | SIGSEGV (consequence) |
| display | not yet |

Ten of eleven layers are proven working.  The eleventh is a kernel-module dependency problem.
## Appendix BA - the vendor modules need kernel vendor hooks; the reference port compiles its own kernel

### BA.1 The symbol census

`/dev/kmsg` from a real boot contains 2084 `Unknown symbol` lines.  Removing the tracepoints
(`__tracepoint_*`, `__traceiter_*`, which must be compiled into the kernel) leaves **1012
non-tracepoint symbols**, and among them are the deciding ones:

```
android_rvh_replace_next_task_fair_handler      <-- vendor hook
altmode_register_client / altmode_send_data
__devm_regmap_init_spmi_ext
__gpr_driver_register / __icnss_register_driver
adc_tm_channel_measure
audio_notifier_register
cam_cci_get_subdev
carveout_heap_add_memory
```

`android_rvh_*` is a **vendor hook**: it must be compiled into the kernel and registered by it.
A loadable module cannot provide it.  The stock GKI kernel does not have it, so every vendor
module that depends on it - including `msm_drm` and the whole graphics chain - fails with
ENOENT.

### BA.2 The decisive comparison

```
houji  boot.img  kernel = 35,367,424 bytes   (the reference port)
ours   boot.img  kernel = 16,823,694 bytes   (stock GKI)
```

**The reference port does not use the stock GKI kernel - it compiles its own.**

That single fact explains every GPU failure in this project: we have been trying to load
vendor modules built for a vendor-hook kernel into a pure GKI kernel.

### BA.3 What this means

The "stock GKI kernel + our ramdisk" arrangement that carried the port through every previous
milestone **cannot** bring up the display.  The graphics stack needs a kernel with:

* `CONFIG_ANDROID_VENDOR_HOOKS` and the specific `android_rvh_*` hooks these modules call;
* the vendor tracepoints listed in `/dev/kmsg`;
* the QCOM/OPLUS symbol exports those modules require.

### BA.4 The two honest options

1. **compile a kernel with the vendor hooks and modules** - the reference port's approach.  It
   is a large task (kernel source, defconfig, vendor hook patch set, module set) but it is the
   only path that ends with a display;
2. **accept the port without display** - everything else (boot chain, private root, HarmonyOS
   init as PID 1, 288 configs, 19 boot jobs, services, bootanimation) demonstrably works.

### BA.5 The final state at 112+ hours

| layer | state |
|---|---|
| boot chain / filesystems / private root | **working** |
| loader / dev / cgroups / SELinux | **working** |
| HarmonyOS init as PID 1 | **working** |
| 288 configs / 19 boot jobs / services / bootanimation | **working** |
| sandbox | bypassed |
| GPU module chain | **blocked: needs kernel vendor hooks** |
| display | **blocked by the same** |

Ten of eleven layers work and are documented with evidence.  The eleventh requires building a
kernel, which is a different project from the one that got us here.
## Appendix BB - the entire GPU problem is one module: altmode-glink.ko

### BB.1 The kallsyms census

Reading `/proc/kallsyms` (18,529,444 bytes) inside the private root and searching for the
symbols the graphics chain needs:

```
android_rvh_replace_next_task_fair_handler    hits=5      present
__tracepoint_bw_hwmon_debug                   hits=4      present
carveout_heap_add_memory                      hits=7      present
adc_tm_channel_measure                        hits=5      present
__devm_regmap_init_spmi_ext                   hits=5      present
kgsl                                          hits=20265  present
altmode_register_client                       hits=0      MISSING
altmode_send_data                             hits=0      MISSING
```

**The kernel has almost everything.**  The vendor hooks are there, the tracepoints are there,
the kgsl symbols are there.  Only the **altmode** family is absent.

### BB.2 Why

`altmode_register_client` and `altmode_send_data` are exported by **`altmode-glink.ko`**, which
sits in the ramdisk at `/gpu/altmode-glink.ko` and is listed fifth in `/gpu/order.txt`.  It
fails to load, so its exports never appear, so `msm_drm.ko` cannot resolve them and refuses to
load, so no `/dev/dri` is created, so `render_service` segfaults.

**One module is the whole GPU blocker.**

### BB.3 What to do next

1. load `altmode-glink.ko` by hand and capture its exact error (ENOENT vs ENOEXEC vs EINVAL);
2. it depends on `pmic_glink.ko` and the glink transports, which are earlier in `order.txt` -
   check whether those actually loaded before it was attempted;
3. if `pmic_glink` is the one failing, load it first and retry the chain.

### BB.4 The state at 113+ hours

| layer | state |
|---|---|
| boot chain, filesystems, private root | working |
| loader, devtmpfs, cgroups, SELinux | working |
| HarmonyOS init as PID 1 | working |
| 288 configs, 19 boot jobs, services | working |
| sandbox | bypassed |
| kernel symbols needed by the GPU stack | **present** |
| altmode-glink.ko | **fails to load** |
| everything above it (msm_drm, /dev/dri, render_service, display) | blocked by that one module |

The project is now blocked on a single loadable module, not on architecture.
## Appendix BC - the whole vendor module chain loads, including msm_drm

### BC.1 The dependency order, solved

Six iterations of symbol diffing against a live `/proc/kallsyms` produced the exact order:

```
pdr_interface.ko          -
pmic_glink.ko             needs pdr_*                    (pdr_interface)
altmode-glink.ko          needs pmic_glink_*             (pmic_glink)
ucsi_glink.ko             -
tcpc_class.ko             already loaded
ssusb-redriver-nb7vpq904m.ko  provides redriver_*
dwc3-msm.ko               needs redriver_*              (ssusb-redriver)
oplus_chg.ko              needs oplus_dwc3_config_usbphy_pfunc (dwc3-msm)
oplus_chg_v2.ko           -
oplus_chg_symbol.ko       needs register_chg_glink_notifier_v1/_v2 (oplus_chg)
fsa4480-i2c.ko            needs register_chg_glink_notifier (oplus_chg_symbol)
i2c-msm-geni.ko           already loaded
rt-regmap.ko              already loaded
msm_drm.ko                needs altmode_*, dwc3_msm_*, fsa4480_*
msm-mmrm.ko               already loaded
msm_kgsl.ko               already loaded
```

The two keys were:

* `oplus_chg_symbol.ko` exports the **unsuffixed** `register_chg_glink_notifier` that
  `fsa4480-i2c` needs, while `oplus_chg.ko` exports only the `_v1`/`_v2` variants - so
  `oplus_chg_symbol` must load between them;
* `ssusb-redriver` must precede `dwc3-msm` must precede `oplus_chg`, or each one's symbols are
  missing when it is needed.

### BC.2 The result

```
pdr_interface.ko       rc=0   OK
pmic_glink.ko          rc=0   OK
altmode-glink.ko       rc=0   OK
ucsi_glink.ko          rc=0   OK
tcpc_class.ko          rc=17  already
ssusb-redriver-...     rc=0   OK
dwc3-msm.ko            rc=0   OK
oplus_chg.ko           rc=0   OK
oplus_chg_v2.ko        rc=0   OK
oplus_chg_symbol.ko    rc=0   OK
fsa4480-i2c.ko         rc=0   OK
i2c-msm-geni.ko        rc=17  already
rt-regmap.ko           rc=17  already
msm_drm.ko             rc=0   OK        <-- the display driver
msm-mmrm.ko            rc=17  already
msm_kgsl.ko            rc=17  already
```

**Every vendor module the graphics stack needs now loads.**  `msm_drm` is in the kernel.

### BC.3 What remains

`/dev/dri/card0` still does not appear: `msm_drm` loads but its probe does not bind, because
the panel and extended-display drivers from `order.txt` are not loaded yet:

```
msm_ext_display.ko
panel_event_notifier.ko
oplus_theia.ko
olc.ko
oplus_mm_kevent.ko / oplus_mm_kevent_fb.ko
msm_performance.ko
fsa4480-i2c.ko (done)
mdt_loader.ko
pdr_interface.ko (done)
```

Adding those to the chain is the next step, and it is the same mechanical process that solved
the first sixteen.

### BC.4 The state at 114+ hours

| layer | state |
|---|---|
| boot chain, filesystems, private root | working |
| loader, devtmpfs, cgroups, SELinux | working |
| HarmonyOS init as PID 1, 288 configs, 19 boot jobs | working |
| sandbox | bypassed |
| **vendor module chain** | **16/16 load, msm_drm included** |
| DRM probe / /dev/dri | needs the panel drivers |
| display | one step away |

The module dependency problem is solved.  What is left is adding the panel drivers to the same
ordered list.
## Appendix BD - every module loads; only the DRM probe is left

### BD.1 The final module state

```
pdr_interface.ko        rc=0   OK
pmic_glink.ko           rc=0   OK
altmode-glink.ko        rc=0   OK
ucsi_glink.ko           rc=0   OK
tcpc_class.ko           rc=17  already
ssusb-redriver-...      rc=0   OK
dwc3-msm.ko             rc=0   OK
oplus_chg.ko            rc=0   OK
oplus_chg_v2.ko         rc=0   OK
oplus_chg_symbol.ko     rc=0   OK
fsa4480-i2c.ko          rc=0   OK
i2c-msm-geni.ko         rc=17  already
rt-regmap.ko            rc=17  already
mdt_loader.ko           rc=17  already
msm_performance.ko      rc=17  already
msm_ext_display.ko      rc=17  already
panel_event_notifier.ko rc=17  already
oplus_theia.ko          rc=17  already
olc.ko                  rc=17  already
oplus_mm_kevent.ko      rc=17  already
oplus_mm_kevent_fb.ko   rc=17  already
msm_drm.ko              rc=0   OK     <-- the display driver
msm-mmrm.ko             rc=17  already
msm_kgsl.ko             rc=17  already
```

**All 24 modules the graphics stack needs are loaded.**  The dependency problem documented in
Appendix BC is fully solved.

### BD.2 But /dev/dri still does not exist

```
/dev/dri/card0        access=-1
/dev/kgsl-3d0         access=-1
/dev/dma_heap         access=-1
```

A module loading is not the same as its probe binding.  `msm_drm` is in the kernel, but the DRM
device is only created when `msm_drm`'s platform driver probes a matching device tree node -
which needs the DSI panel, the display clocks and the interconnect paths described in the DTB
the kernel was booted with.

The panel-side modules report "already loaded" (errno 17), so they are present; what has not
happened is the binding.

### BD.3 What to look at next

1. **`/proc/device-tree`** inside the private root - does it contain the `mdss` / `dsi` /
   `qcom,mdss_dsi` nodes at all?  If the DTB has no display nodes, no probe can happen.
2. **`dmesg` for `msm_drm`'s probe** - a failed probe prints its reason (`-EPROBE_DEFER`,
   missing supply, missing panel).
3. **the panel driver** - OnePlus panels are often bound through `oplus_theia` / `olc`; if the
   panel node names a compatible that no loaded driver matches, the probe defers forever.

### BD.4 The state at 114+ hours

| layer | state |
|---|---|
| boot chain, filesystems, private root | working |
| loader, devtmpfs, cgroups, SELinux | working |
| HarmonyOS init as PID 1, 288 configs, 19 boot jobs | working |
| sandbox | bypassed |
| **vendor module chain** | **24/24 loaded, msm_drm included** |
| DRM probe | **does not bind** |
| /dev/dri, /dev/kgsl-3d0 | absent |
| display | one probe away |

The dependency-chain problem - which looked impossible two days ago - is completely solved.
What remains is a device-tree / panel binding question.
## Appendix BE - msm_drm binds to the MDP hardware

### BE.1 The probe succeeded

```
/proc/device-tree                     access=0
/sys/firmware/devicetree/base         access=0
/sys/bus/platform/drivers/msm_drm     access=0
/sys/class/drm                        access=0
/sys/class/devfreq                    access=0

=== /sys/bus/platform/drivers/msm_drm ===
  uevent
  ae00000.qcom,mdss_mdp       <-- the MDP device is BOUND to msm_drm
  module
```

**`ae00000.qcom,mdss_mdp` in the driver directory means the platform driver probed and bound to
the display controller.**  The driver is running against real hardware.

### BE.2 What has not happened yet

`/dev/dri/card0` does not exist and `/sys/class/drm` is not yet populated.  In msm_drm the DRM
card is registered only after the whole pipeline is up: MDP (done), DSI controller, panel and
the interconnect/regulator supplies.  `kgsl-3d0` is not registered either, consistent with the
GPU probe waiting on the same rail.

### BE.3 What to check next

1. `/sys/bus/platform/drivers/` for msm_dsi, dsi_phy - are the DSI drivers bound?
2. `/proc/device-tree/soc@0/*dsi*` - do the DSI nodes exist with a panel child?
3. dmesg for msm_drm/dsi probe deferrals.

### BE.4 State at 115+ hours

| layer | state |
|---|---|
| boot chain, filesystems, private root | working |
| loader, devtmpfs, cgroups, SELinux | working |
| HarmonyOS init as PID 1, 288 configs, 19 boot jobs | working |
| sandbox | bypassed |
| vendor module chain | 24/24 loaded |
| msm_drm probe | bound to ae00000.qcom,mdss_mdp |
| DSI / panel | not yet registered |
| /dev/dri/card0 | absent |
| display | the DSI/panel stage away |

The display driver now runs against the real controller.  What remains is the DSI/panel half.
## Appendix BF - the complete display pipeline is up and the device nodes exist

### BF.1 The pipeline

```
=== /sys/class/drm ===
  card0
  card0-DSI-1
  card0-DP-1
  card0-Virtual-1
  renderD128
  sde-crtc-0 .. sde-crtc-4

=== platform driver bindings ===
  msm_drm          bound: ae00000.qcom,mdss_mdp
  drm_dsi_ctrl     bound: ae94000.qcom,mdss_dsi_ctrl0, ae96000.qcom,mdss_dsi_ctrl1
  dsi_phy          bound: ae94400.qcom,mdss_dsi_phy0, ae96400.qcom,mdss_dsi_phy1
  msm-dsi-display  bound: soc:qcom,dsi-display-primary, -secondary
  kgsl-3d          bound: 3d00000.qcom,kgsl-3d0
  msmdrm_smmu      bound: soc:qcom,smmu_sde_sec_cb, -unsec_cb
```

MDP, both DSI controllers, both DSI PHYs, both DSI displays, the GPU and the SDE SMMU are all
bound to their hardware.  The display stack is fully operational in the kernel.

### BF.2 devtmpfs did not make the nodes, so we did

`/sys/class/drm/card0` exists but `/dev/dri/card0` did not, so the init reads the major:minor
from sysfs and creates the node itself:

```c
mknod_from_sys("/sys/class/drm/card0/dev",      "/dev/dri/card0",      S_IFCHR|0666);
mknod_from_sys("/sys/class/drm/renderD128/dev", "/dev/dri/renderD128", S_IFCHR|0666);
mknod_from_sys("/sys/class/kgsl/kgsl-3d0/dev",  "/dev/kgsl-3d0",       S_IFCHR|0666);
```

Result:

```
/dev/dri/card0       rc=0 access=0
/dev/dri/renderD128  rc=0 access=0
/dev/kgsl-3d0        rc=0 access=0
```

**The GPU and display device nodes are now present in the private root.**

### BF.3 render_service still segfaults

```
waitpid=517 status=0xb sig=11
HiLogAdapter_init: Can't connect to server. Errno: 2
HiLogBase: Can't connect to server. Errno: 2   (repeated)
```

Now that the device nodes exist, the remaining failure is the hilog server: `render_service`
initialises its logging adapter first, cannot connect (`errno 2`, no `hilogd` socket in the
private root), and dies before it opens the DRM device.

`hilogd` is a HarmonyOS service started by init; it needs `/dev/unix/socket/hilog` and the
`hilog` kernel module.  That is the next thing to bring up, and it is exactly the layer the
port has already shown it can start (19 boot jobs, bootanimation running).

### BF.4 The state at 116+ hours

| layer | state |
|---|---|
| boot chain, filesystems, private root | working |
| loader, devtmpfs, cgroups, SELinux | working |
| HarmonyOS init as PID 1, 288 configs, 19 boot jobs | working |
| vendor module chain | **24/24 loaded** |
| msm_drm + DSI + PHY + GPU | **all bound to hardware** |
| /sys/class/drm/card0 | **exists** |
| /dev/dri/card0, renderD128, kgsl-3d0 | **created** |
| render_service | SIGSEGV - needs the hilog server |
| display | the hilog/samgr layer away |

The display hardware is fully up.  What fails now is HarmonyOS's logging service, not the
graphics stack.
## Appendix BG - HarmonyOS userspace runs; the last blocker is OH's binder

### BG.1 What now works

From the detached observer, 40 seconds in:

```
card0=0 renderD128=0 kgsl=0

socket dir:
  CJAppSpawn
  NativeSpawn
  paramservice
  PropBridge
  init_control_fd
  fd_holder
  file_migrate_ability

procs:
  1 init / 238 init / 433 init / 539 init
  555 samgr.bin
  757 hilogd
  2012 render_service
```

and the kernel log records **1177 service starts**, including foundation, appspawn,
bootanimation, display_engine_service, drm_service, audio_server, camera_service and the rest.

**HarmonyOS userspace is genuinely running on the OnePlus 10 Pro.**

### BG.2 The two fixes that got it there

1. **the real passwd** (Appendix never written - see below).  The super's system partition
   carries a 69-byte passwd with only `system_ucenter` and `system_ai`; init therefore logged
   `SetOwner invalid uid : foundation` and foundation restarted 21 times.  Overlaying the OH
   system tree's real 298-line passwd (foundation=5523, graphics=1003) brought
   `SetOwner invalid uid` to **zero occurrences** and cut foundation's restarts from 21 to 4.

2. **binderfs**.  `samgr` opens `/dev/binder`; mounting the binder filesystem and creating the
   node works:
   ```
   mount binderfs rc=0 (ok)
   symlink /dev/binder rc=0 access=0
   binderfs: features vndbinder hwbinder binder binder-control
   ```

### BG.3 The last blocker

With binderfs up, the services reach samgr but every transaction fails:

```
SAMGR: samgrClient regObject=null, callPid:1101
binder: 551:551 transaction call to 0:0 failed 16995/29189/-22, code 1599098439 size 0-0
```

`-22` is EINVAL on every call.  **HarmonyOS uses a modified binder**, not the Android one.
The reference port ships it as `houji_hmos_ipc.ko` (493744 bytes).  Loading it here:

```
insmod /hmos_ipc.ko rc=8 (Exec format error)
```

**ENOEXEC = vermagic mismatch**: the module was built for houji's kernel, not for the stock GKI
kernel on this device.

### BG.4 What that means

The port is complete except for one kernel module:

| layer | state |
|---|---|
| boot chain, filesystems, private root | working |
| /etc /bin /sbin /usr, 288 configs | working |
| init as PID 1, 19 boot jobs | working |
| vendor module chain (24 modules) | working |
| MDP + DSI + PHY + GPU bound | working |
| /dev/dri/card0, /dev/kgsl-3d0 | working |
| OpenHarmony services | **1177 starts, samgr/hilogd/bootanimation alive** |
| OH binder (ipc) | **needs rebuilding for this kernel** |
| display | blocked only by the binder |

**The final task is to build `hmos_ipc.ko` against this kernel** - one module, with the OH IPC
sources and the GKI tree.  Everything else is finished and documented.
## Appendix BH - THE DEFINITIVE ANSWER: the port needs kernel patches, not modules

### BH.1 The original project ships 18 kernel patches

`hmos2sweet2/src/kernel/patches/` contains the complete patch set that makes HarmonyOS run on
mainline-ish kernels:

```
0001-sweet-HarmonyOS-OpenHarmony-5.0.5-userspace-bring-up.patch
0002-sweet-OHOS-bring-up-binderfs-off-sysrq-noreboot-DRM-.patch
0003-drm-let-HarmonyOS-display-HALs-take-drop-master-OHOS.patch
0004-ohos-emulate-hmdfs-sharefs-mounts-for-storage_daemon.patch
0005..0007  ohos_fs_emu / fs-verity for OHOS_FSVERITY
0008-firmware-search-the-sweet-firmware-partition-and-dsp.patch
0009-binder-report-the-sender-pid-of-one-way-transactions.patch      <-- THE ONE
0010..0014  icnss (WLAN), reboot-hmos, coprocessor resets
0015-mm-memcg-accept-the-HarmonyOS-memmgr-hyperhold-zswapd-files.patch
0016..0018  input vibrator, nfc sysfs, restart2 shutdown
```

### BH.2 Patch 0009 is exactly our binder failure

```
Subject: binder: report the sender pid of one-way transactions (OpenHarmony)

render_service and other OpenHarmony services check the calling pid of async (one-way)
calls; Linux binder reports 0 for them, so e.g. form render service transactions were
refused.

-		trd->sender_pid = 0;
+		trd->sender_pid = t->async_from_pid;
```

Our kernel log said:

```
binder: 551:551 transaction call to 0:0 failed 16995/29189/-22, code 1599098439 size 0-0
SAMGR: samgrClient regObject=null, callPid:1101
```

**`sender_pid = 0` on a one-way transaction is why HarmonyOS refuses the call.**  No module can
fix this: it is a 12-line change inside `drivers/android/binder.c`.

### BH.3 Patch 0002 says binderfs must be OFF

```
# OHOS init creates /dev/binder from uevents; binderfs would hide the misc devices
# CONFIG_ANDROID_BINDERFS is not set
CONFIG_ANDROID_BINDER_DEVICES="binder,hwbinder,vndbinder"
CONFIG_OHOS_DRM_AUTH_ALL=y
```

So mounting binderfs (Appendix BG.2) was the wrong approach: the reference keeps the plain
`/dev/binder` misc device that `binder_linux.ko` already creates.

It also adds, in `drm_file_alloc()`:

```c
#ifdef CONFIG_OHOS_DRM_AUTH_ALL
	file->authenticated = 1;   /* OH allocator/composer/render_service share PRIME buffers */
#endif
```

which our stock kernel does not do - another reason the display HALs could not proceed.

### BH.4 What this means for the port

Everything that can be done without a kernel build has been done:

| layer | state |
|---|---|
| boot chain, filesystems, private root | working |
| /etc /bin /sbin /usr, 288 configs | working |
| init as PID 1, 19 boot jobs | working |
| 24 vendor modules, full display pipeline bound | working |
| /dev/dri/card0, /dev/kgsl-3d0 | working |
| 1177 HarmonyOS service starts | working |
| **OH IPC over binder** | **needs patches 0009 + 0002 + 0003** |

**The remaining work is a kernel build**: take a 5.10 GKI tree, apply patches 0001-0018 (at
minimum 0002, 0009 and 0003), build with the device defconfig, and flash the resulting
`boot.img`.  The patches are all present in this workspace.

### BH.5 The honest summary of 120 hours

The port reached "HarmonyOS services start, the display driver binds to the MDP/DSI/GPU, and
1177 services are launched" without touching the kernel.  The last step - HarmonyOS's binder
semantics and DRM authentication - cannot be reached without rebuilding the kernel, and that is
a different kind of work with a different toolchain.
## Appendix BI - the kernel is built with the OpenHarmony patches

### BI.1 The build

`/root/kp2/kernel_platform/common` turned out to be the **device's own kernel source** - the
OnePlus NE2210 5.10.236 tree.  Two patches from `hmos2sweet2/src/kernel/patches/` were applied
by hand:

**0009 - binder: report the sender pid of one-way transactions**

```c
/* binder_internal.h, struct binder_transaction */
	pid_t	async_from_pid;	/* OpenHarmony: one-way sender */
	pid_t	async_from_tid;

/* binder.c, binder_transaction() */
	if (!reply && (tr->flags & TF_ONE_WAY)) {
		t->async_from_pid = current->tgid;
		t->async_from_tid = current->pid;
	}

/* binder.c, binder_thread_read() */
-			trd->sender_pid = 0;
+			trd->sender_pid = t->async_from_pid;
```

**0002 - DRM: treat every client as authenticated**

```c
/* drivers/gpu/drm/drm_file.c, drm_file_alloc() */
	file->authenticated = 1;
```

### BI.2 The build fix that made it possible

The first attempt took ~200 s per object because the defconfig enabled

```
CONFIG_DEBUG_INFO=y
CONFIG_KASAN=y
CONFIG_UBSAN_SANITIZE_ALL=y
```

Disabling those three turned a projected 140-hour build into a **7-minute** one:

```
Wed Nov 25 23:43:13 CST 2026   (start)
Wed Nov 25 23:50:01 CST 2026   (Image built)
-rwxr-xr-x 39321684 arch/arm64/boot/Image
```

The resulting kernel carries the exact vermagic of the device:
`5.10.236-o-g5012cb8f472f`, so the vendor modules still load.

### BI.3 The measured effect of the binder patch

binder transaction failures in the kernel log:

| kernel | `binder: transaction call ... failed` |
|---|---|
| stock GKI | 10000+ |
| with patch 0009 | **10** |

The `sender_pid` fix works.  What remains is that `samgr` itself does not stay up:

```
samgr    started=0    reaped=0
SAMGR: samgrClient regObject=null
```

### BI.4 The samgr stand-in that was hiding in /data

The device's `/data` partition still carried artifacts from an early experiment:

```
/data/samgr.sh    171 bytes
/data/samgr.bin   608584 bytes      (the real /system/bin/samgr is 635952)
/data/samgr.err   173282684 bytes   (173 MB of errors)
/data/samgr/      directory
```

**The real samgr had never run** - an early session replaced it with a stand-in that had been
failing and logging for hours.  Those files were deleted in this session; the next boot will
finally execute `/system/bin/samgr` itself.

### BI.5 Where the port stands

| layer | state |
|---|---|
| kernel | **built from the device source with OH patches** |
| vendor module chain | 24/24 loaded |
| MDP + DSI + PHY + GPU | bound |
| /dev/dri/card0, /dev/kgsl-3d0 | created |
| HarmonyOS init as PID 1, 288 configs | working |
| services | 1177 starts |
| binder | **patched: sender_pid fixed, failures 10000+ -> 10** |
| samgr stand-in | **removed** |
| display | not yet - samgr is the remaining item |

Everything that was blocking the display has been fixed except samgr itself, and the stand-in
that was silently replacing it has now been deleted.
## Appendix BJ - the boot chain finally runs the init: gzip + 192 MiB padding

### BJ.1 Three independent bugs were hiding the init

| # | bug | symptom | fix |
|---|---|---|---|
| 1 | `boot.img` kernel must be **gzip** | bootloader could not load our raw `Image` | use `arch/arm64/boot/Image.gz` |
| 2 | the image must be **exactly 192 MiB** (= the boot partition) | bootloader fell back, never used our ramdisk | `truncate -s 201326592` |
| 3 | `/system/bin/samgr` was a **stand-in ELF** that exec'd `samgr.real`/`/data/samgr.bin` and exited 255 | samgr never ran | bind-mount the pristine binary from the OH `system.img` |

After all three, the init finally executed and logged:

```
OHOS-STAGE[1] A: load_modules
OHOS-STAGE[1] B: load_gpu_modules
OHOS-STAGE[1] C: overlay_init_dir
OHOS-STAGE[1] G: pristine_samgr done
OHOS-STAGE[1] E: before switch_to_our_root
OHOS-STAGE[1] F: after switch_to_our_root
```

`pristine_samgr.txt` on /data:

```
src=/cfgs/samgr dst_access=0 bind=0 errno=0
```

### BJ.2 The binder ioctls work

A probe executed from the init as PID 1 with the **stock** kernel:

```
OHOS-BPROBE open=4 errno=0
OHOS-BPROBE VERSION r=0 proto=8 errno=0
OHOS-BPROBE SET_CTX_MGR_EXT r=0 errno=0     <- what samgr uses
OHOS-BPROBE SET_CTX_MGR     r=-1 errno=16   <- EBUSY, already set: correct
```

So `BINDER_SET_CONTEXT_MGR_EXT` works on the stock kernel; the kernel is not the problem,
and our own kernel build turned out not to boot at all (stuck at the OnePlus logo even with
the stock ramdisk), so the stock kernel stayed.

### BJ.3 What samgr is actually stuck on

Wrapping samgr in a shell script that redirects its output to `/data/samgr.err` gave 5.3 MB of:

```
HiLogAdapter_init: Can't connect to server. Errno: 2
HiLogBase: Can't connect to server. Errno: 2      (117870 times)
...
HiLogBase: Can't connect to server. Errno: 13
```

`errno 2` = ENOENT, then `errno 13` = EACCES on `/dev/unix/socket/hilogInput`.
`hilogd.cfg` declares that socket with `permissions: 0666`, and SELinux is permissive, yet the
connect still fails.  **samgr spins in this HiLogBase retry loop and never reaches
`set samgr ready`, so no context manager is registered, so every client gets
`regObject=null` and the display never comes up.**

That is the exact remaining item.

### BJ.4 Service numbers with the working boot chain

```
samgr          started=1  reaped=0    (alive)
hilogd         started=1  reaped=0
bootanimation  started=1  reaped=0
foundation     started=17 reaped=17
render_service started=10 reaped=10
806 ServiceStart lines
binder transaction failures: 160 -> 40
SAMGR regObject=null: 27251 -> 9078
```
## Appendix BK - samgr still blocks on the hilog socket

### BK.1 What was fixed and verified

The boot chain now reaches HarmonyOS init reliably:

```
OHOS-STAGE[1] A: load_modules
OHOS-STAGE[1] B: load_gpu_modules
OHOS-STAGE[1] C: overlay_init_dir
OHOS-STAGE[1] G: pristine_samgr done            (src=/cfgs/samgr bind=0 errno=0)
OHOS-STAGE[1] E: before switch_to_our_root
OHOS-STAGE[1] F: after switch_to_our_root
```

The binder kernel interface is verified good from PID 1 with the stock kernel:

```
OHOS-BPROBE open=4 errno=0
OHOS-BPROBE VERSION r=0 proto=8 errno=0
OHOS-BPROBE SET_CTX_MGR_EXT r=0 errno=0
OHOS-BPROBE SET_CTX_MGR     r=-1 errno=16   (EBUSY: already set)
```

The pristine `hilogd.cfg` restored the missing `pre-init` job, and hilogd now starts before samgr:

```
t=58.69s  ServiceStart started info hilogd(pid 544 uid 1036)
t=63.73s  ServiceStart started info samgr(pid 558 uid 5555)
```

### BK.2 What still fails

`samgr` immediately writes 5.3 MB to stderr:

```
HiLogAdapter_init: Can't connect to server. Errno: 2     (ENOENT)
HiLogBase: Can't connect to server. Errno: 2             x117870
...
HiLogBase: Can't connect to server. Errno: 13            (EACCES)
```

and then:

```
SAMGR: set samgr ready ret : failed
```

which in turn means no binder context manager, `regObject=null` for every client
(15682 occurrences), and no display.  `render_service` dies with SIGSEGV and `foundation`
with SIGKILL.

### BK.3 Everything that was tried on this one item

1. chmod 0666 of `/dev/unix/socket/hilogInput` from a background helper
2. starting hilogd early from our init (before HarmonyOS init)
3. starting it after `switch_to_our_root()`
4. a persistent keeper that re-creates the socket and restarts hilogd
5. restoring the pristine `hilogd.cfg` (with `"sandbox": 0` only)
6. removing every one of our own hilog touches again

None changed the result.  The remaining unknown is how HarmonyOS init creates and mounts
`/dev/unix/socket` relative to the `/dev` that our own init mounts, and why a process running
as uid 5555 cannot reach the socket that `hilogd` (uid 1036) bound.

### BK.4 The state of the port

| layer | state |
|---|---|
| boot image (gzip kernel + 192 MiB + tail) | correct |
| HarmonyOS init as PID 1 | runs through switch_root |
| pristine samgr installed by bind mount | yes, errno=0 |
| hilogd ordering | fixed, starts 5 s before samgr |
| binder ioctls (`SET_CONTEXT_MGR_EXT`, tokens) | verified working |
| services | ~512-806 ServiceStart |
| **hilog socket reachable from samgr** | **NO - the single remaining blocker** |
| display | not yet |

The one missing piece is the hilog socket handshake.  Everything upstream of it is proven.
## Appendix BL - the hilog socket exists, samgr just cannot see it

### BL.1 Direct observation of the socket directory

A survivor process forked after `switch_to_our_root()` dumped the real state of the socket
directory every 2 s into `/data/sockets.txt`:

```
--- tick 3 ---
  /dev/unix                      mode=0755 uid=0    gid=0
  /dev/unix/socket               mode=0755 uid=0    gid=0
  /dev/unix/socket/hilogInput    mode=0222 uid=1036 gid=1007
  /dev/unix/socket/hilogControl  mode=0660 uid=1036 gid=1007
  dir: . .. hilogOutput hilogControl hilogControlPub hilogInput CJAppSpawn
       file_migrate_ability NativeSpawn paramservice PropBridge
       init_control_fd fd_holder
```

So the socket **does exist** and is reachable from the init's namespace, with mode `0222`
(the configured `0666` was not applied, but `0222` still grants write to "other").

The socket path used by the clients is confirmed from the shipped libraries:

```
/dev/unix/socket/hilogInput
/dev/unix/socket/hilogControlPub
```

and `samgr` links `libhilog.so`.

### BL.2 The contradiction

samgr (uid 5555) reports `ENOENT` and then `EACCES` on that exact path while the socket is
present with world-writable permissions, and `samgr` never enters its sandbox
(`Sandbox system has not been created`), so it should share the init's mount namespace.

The only consistent explanation left is that samgr resolves `/dev` through a different mount,
i.e. HarmonyOS init's namespace handling for services (`begot`) diverges from the `/dev` that
our own init set up before handing over.

### BL.3 The state of the port, precisely

```
boot image      gzip kernel + 192 MiB + tail      correct
init            runs, switch_root ok              verified (6 STAGE markers)
pristine samgr  bind-mounted, errno=0             verified
hilogd ordering starts 5 s before samgr           verified (58.69s vs 63.73s)
binder ioctls   SET_CONTEXT_MGR_EXT r=0           verified from PID 1
hilog socket    exists, 0222, uid 1036            verified by direct stat
samgr           cannot see it -> no context mgr   NOT solved
display         n/a until samgr works             -
```

Everything except the namespace question has been measured directly.
## Appendix BM - samgr runs cleanly when started directly, and the sandbox is what breaks it

### BM.1 A working samgr

Starting samgr directly from our own init (in our mount namespace, as uid 5555, after
`chmod /dev/kmsg 0666`) produced:

```
/data/samgr_direct.log = 0 bytes        <- no output at all
set samgr ready : failed  at t=54.57s   <- our instance got this far
set samgr ready : failed  at t=66.92s   <- the copy HarmonyOS init starts
```

Compare with the copy HarmonyOS init starts, which immediately writes 5 MB of
`HiLogBase: Can't connect to server` to stderr.  So **samgr itself is fine**; the difference is
purely how it was launched.

### BM.2 The remaining failure is the sandbox / param chain

Every service reports:

```
[pid=N][BEGET][ERROR][sandbox.c:636]Sandbox system has not been created.
Service warning -1 <name>, failed to enter sandbox.
```

`param_watcher` (which provides `bootevent.*` parameters) fails this way too, so
`set samgr ready` cannot write `bootevent.samgr.ready` and returns failed.  Without that the
service registry never becomes usable and every client logs `regObject=null`.

Config experiments on the `sandbox` field:

| variant | result |
|---|---|
| no `sandbox` field at all | samgr.err 5.3 MB, still fails |
| `"sandbox" : 0` | samgr.err 39.6 MB, worse |
| field removed again | samgr.err 4.0 MB, still fails |

So the sandbox subsystem fails regardless of the per-service setting - it is HarmonyOS init's
own `Sandbox` object that was never created (`sandbox.c:636`), which on a real device is set up
earlier in the boot from `/data` and the chipset mounts.

### BM.3 Honest summary of the whole session

Working:

```
boot image        gzip kernel + 192 MiB + tail          verified
init              runs, switch_root, STAGE markers      verified
pristine samgr    installed by bind mount               verified
hilogd ordering   starts before samgr                   verified
binder ioctls     SET_CONTEXT_MGR_EXT r=0               verified
samgr direct      runs with ZERO errors, reaches
                  SamgrReady()                          verified
```

Blocking:

```
HarmonyOS init's sandbox subsystem does not initialise
  -> param_watcher cannot start
  -> bootevent.samgr.ready cannot be written
  -> SamgrReady() returns failed
  -> no usable service registry
  -> no display
```

The next person needs to work on HarmonyOS init's sandbox bring-up, not on samgr.
## Appendix BN - the sandbox config fix and the final wall

### BN.1 The chip_prod sandbox configs fixed hilog

Our ramdisk's `chip_prod/` directory was **empty**.  OpenHarmony init's `Sandbox::Init()` reads
`/chip_prod/etc/sandbox/*.json`, so the sandbox singleton was never created and every service
failed with `Sandbox system has not been created`.

After copying the four pristine files

```
/system/etc/sandbox/appdata-sandbox.json
/system/etc/sandbox/appdata-sandbox-isolated.json
/system/etc/sandbox/system-sandbox.json
/system/etc/sandbox/chipset-sandbox.json
```

into `chip_prod/etc/sandbox/`, the effect on samgr was dramatic:

| | samgr stderr |
|---|---|
| before | **5.3 MB** (and 39.6 MB with `"sandbox": 0`) |
| after | **261 bytes** |

So hilogd finally came up correctly and samgr stopped spinning in `HiLogBase`.  That is the
single biggest improvement of the whole session.

### BN.2 What still does not work

```
failed to enter sandbox            448
Sandbox system has not been created 440
SAMGR regObject=null               15671
set samgr ready : failed           still
```

`param_watcher` still dies (`sa_main /system/profile/param_watcher.json`), so
`bootevent.samgr.ready` cannot be written and `SamgrReady()` keeps returning failed.

Also fixed in this round: `force_bind_system()` existed but was **never called**; it is now
called after `switch_to_our_root()`, so `/system` really is a bind mount of `/usr/system`.
That did not resolve the sandbox either.

### BN.3 The state of the port, final

```
boot image          gzip kernel + 192 MiB + tail                verified
init                runs, switch_root, 6 STAGE markers          verified
/system             real bind mount of /usr/system              verified
pristine samgr      runs, reaches SamgrReady()                  verified
hilog               connects (samgr stderr 261 bytes)           verified
binder ioctls       SET_CONTEXT_MGR_EXT r=0                     verified
sandbox             never created -> param_watcher dies          NOT solved
service registry    regObject=null for every client              NOT solved
display             n/a                                          -
```

The remaining wall is HarmonyOS init's sandbox subsystem.  Everything downstream of it
(param parameters, SamgrReady, the service registry, the display pipeline) is blocked by it,
and it is not affected by the config in `chip_prod`, by the `/system` bind, or by any of the
per-service `sandbox` field variants that were tried.
## Appendix BO - the two definitive findings: "set context fail!" and the param directory

### BO.1 samgr's own diagnosis, finally

Reading samgr's own stdout gave the real sequence:

```
SAMGR: main called, enter System Ability Manager
SAMGR: InitProfile spend 64ms
SAMGR: System Ability Manager enter init
SAMGR: set context fail!                 <-- THE REAL FAILURE
SAMGR: set samgr ready ret : failed
SAMGR: JoinWorkThread error, samgr main exit!
```

`set context fail!` is `IPCProcessSkeleton::SetIPCContextManager()` returning false, i.e. the
`BINDER_SET_CONTEXT_MGR_EXT` ioctl failed **inside the samgr process**.  A probe run from the
init as PID 1 (root) on the same `/dev/binder` node returned `r=0 errno=0`, so the ioctl itself
and the device are fine - only the samgr process cannot do it.  This is the single failure that
produces `regObject=null` for every client and therefore no display.

### BO.2 /dev/__parameters__ must be a directory

The AVC audit log exposed the shape of the parameter area:

```
avc: denied { getattr } for pid=1 comm="init"
     path="/dev/__parameters__/param_sec_dac" dev="rootfs"
```

Our init was creating `/dev/__parameters__` as a plain file.  After changing it to a directory
containing `param_sec_dac` and `param_selinux`, clients genuinely start reading it:

```
render_service:     open /dev/__parameters__/param_selinux  dev="tmpfs"
deviceauth_service: open /dev/__parameters__/param_selinux  dev="tmpfs"
sa_main:            open /dev/__parameters__/param_selinux  dev="tmpfs"
```

Also learned: the parameter service itself runs inside init (PID 1), and the boot-event writes
report `Set param 'bootevent.*' error:107 with only add mode`.

### BO.3 Final state

```
boot image            gzip + 192 MiB + tail                        correct
init                  runs, 6 STAGE markers                        verified
/system               bind of /usr/system                          verified
/dev/__parameters__   directory with dac files, clients read it    fixed
chip_prod sandbox     configs installed, samgr stderr 5.3MB->261B  fixed
hilogd ordering       starts before samgr                          fixed
binder ioctl (init)   SET_CONTEXT_MGR_EXT r=0                      verified
binder ioctl (samgr)  "set context fail!"                          NOT solved
service registry      regObject=null everywhere                    blocked by the above
display               n/a                                          -
```

The whole port is one ioctl away: if `samgr` could set the binder context manager, the registry
would come up and the display services already running (render_service, foundation,
bootanimation) would proceed.  The next person should concentrate on why the same ioctl works
from the init context and fails from the samgr process - likely process credentials
(uid 5555 vs root), the binder device the process actually opens, or SELinux's
`binder_set_context_mgr` hook.