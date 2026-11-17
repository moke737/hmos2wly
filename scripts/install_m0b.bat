@echo off
setlocal
set ADB=E:\1+10pro\_flashtool\adb.exe
set OTA=E:\1+10pro\1+10pro[HMOS]\m0b\ota
set SUP=/dev/block/by-name/super

echo === push metadata + lp_probe ===
%ADB% push "E:\1+10pro\1+10pro[HMOS]\m0b\lp_hmod_slot.bin" /tmp/lp.bin
%ADB% push "E:\1+10pro\1+10pro[HMOS]\m0b\lp_probe" /tmp/lp_probe
%ADB% shell chmod 755 /tmp/lp_probe

echo === write metadata (slot 0 @0x3000, slot 1 @0x13000) ===
%ADB% shell "dd if=/tmp/lp.bin of=%SUP% bs=4096 seek=3 count=16 conv=notrunc"
%ADB% shell "dd if=/tmp/lp.bin of=%SUP% bs=4096 seek=19 count=16 conv=notrunc"

echo === stream images ===
echo -- system (seek 1)
%ADB% shell "dd of=%SUP% bs=1M seek=1 conv=notrunc" < "%OTA%\system.img"
echo -- sys_prod (seek 2953)
%ADB% shell "dd of=%SUP% bs=1M seek=2953 conv=notrunc" < "%OTA%\sys_prod.img"
echo -- vendor (seek 4721)
%ADB% shell "dd of=%SUP% bs=1M seek=4721 conv=notrunc" < "%OTA%\vendor_negroni.img"
echo -- chip_prod (seek 5364)
%ADB% shell "dd of=%SUP% bs=1M seek=5364 conv=notrunc" < "%OTA%\chip_prod.img"
echo -- cust (seek 5917)
%ADB% shell "dd of=%SUP% bs=1M seek=5917 conv=notrunc" < "%OTA%\cust.img"
echo -- patch (seek 5923)
%ADB% shell "dd of=%SUP% bs=1M seek=5923 conv=notrunc" < "%OTA%\patch.img"

echo === sync + verify ===
%ADB% shell "sync"
%ADB% shell "/tmp/lp_probe"

echo DONE
endlocal
