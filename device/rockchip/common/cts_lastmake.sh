#! /bin/bash
echo "star pre mkimg.."
OUT_FRAMEWORK=out/target/product/rk3026/system/framework/
CORE_JUNIT_JAR=device/rockchip/common/core-junit.jar


#preinstall_path="out/target/product/rk3026/system/preinstall"
#srcapk_file="./device/rockchip/common/app/apk/gl2jni_laohuaceshi.apk"
#srcapk_file1="./device/rockchip/common/app/apk/DeviceTest_Rk_42.apk"
#dstapk_file="out/target/product/rk3026/system/preinstall/gl2jni_laohuaceshi.apk"
#if [ ! -d "$preinstall_path" ];then
#	mkdir $preinstall_path
#	cp $srcapk_file $dstapk_file
#	cp $srcapk_file1 $preinstall_path
#else if [ ! -f "$dstapk_file" ];then
#	cp $srcapk_file $dstapk_file
#	cp $srcapk_file1 $preinstall_path
#fi
#fi
#
#sed -i 's/ro.sf.lcd_density=160/ro.sf.lcd_density=120/' out/target/product/rk3026/system/build.prop
#sed -i 's/ro.secure=1/ro.secure=0/' out/target/product/rk3026/root/default.prop 
#sed -i 's/ro.debuggable=0/ro.debuggable=1/' out/target/product/rk3026/root/default.prop 
#set -i 's/persist.sys.usb.config=none/persist.sys.usb.config=adb/' out/target/product/rk3026/root/default.prop
#echo "persist.service.adb.enable=1" >> out/target/product/rk3026/system/build.prop

#libstagefright_soft_aacdec.so only in 4.2.2 TV
#cp device/rockchip/rk3026/mtv/libstagefright_soft_aacdec.so out/target/product/rk3026/system/lib/

echo "cp core-junit.jar and rm core-junit.odex"
echo "cp $CORE_JUNIT_JAR into $OUT_FRAMEWORK"
cp $CORE_JUNIT_JAR $OUT_FRAMEWORK
find out/target/product/rk3026 -name core-junit.odex -exec rm -f {} \;
echo ""

source build/envsetup.sh 
 mmm packages/apps/Calendar/

find out/target/product/ -name PinyinIME.apk -exec rm {} \;
find out/target/product/ -name PinyinIME.odex -exec rm {} \;

find out/target/product/ -name OpenWnn.apk -exec rm {} \;
find out/target/product/ -name OpenWnn.odex -exec rm {} \;

echo "rm gl2jni_laohuaceshi.apk"
find out/target/product/ -name gl2jni_laohuaceshi.apk -exec rm {} \;
echo "finish pre mkimg"

