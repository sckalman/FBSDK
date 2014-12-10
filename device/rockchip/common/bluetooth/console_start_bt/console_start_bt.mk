
# Start bluetooth from console
PRODUCT_COPY_FILES += \
	device/rockchip/common/bluetooth/console_start_bt/brcm_patchram_plus:system/bin/brcm_patchram_plus \
	device/rockchip/common/bluetooth/console_start_bt/hciconfig:system/xbin/hciconfig \
	device/rockchip/common/bluetooth/console_start_bt/hcidump:system/xbin/hcidump \
	device/rockchip/common/bluetooth/console_start_bt/hcitool:system/xbin/hcitool \
	device/rockchip/common/bluetooth/console_start_bt/libbluedroid.so:system/lib/libbluedroid.so \
	device/rockchip/common/bluetooth/console_start_bt/libbluetooth.so:system/lib/libbluetooth.so \
	device/rockchip/common/bluetooth/console_start_bt/libbluetoothd.so:system/lib/libbluetoothd.so \
	device/rockchip/common/bluetooth/console_start_bt/libbtio.so:system/lib/libbtio.so \
	device/rockchip/common/bluetooth/console_start_bt/libglib.so:system/lib/libglib.so

