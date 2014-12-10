CUR_PATH := device/rockchip/common/app
PRODUCT_PACKAGES += \
	MediaFloat \
	RkApkinstaller \
	RkExplorer \
	RkVideoPlayer \
    StressTest \
	DeviceTest_Rk-20140509 \
	gl2jni_laohuaceshi

PRODUCT_COPY_FILES += \
    $(CUR_PATH)/apk/libgl2jni.so:system/lib/libgl2jni.so
