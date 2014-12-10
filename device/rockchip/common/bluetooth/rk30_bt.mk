CUR_PATH := device/rockchip/common/bluetooth

ifeq ($(strip $(BLUETOOTH_USE_BPLUS)),true)
PRODUCT_PACKAGES += \
	libbt-client-api \
	com.broadcom.bt \
	com.broadcom.bt.xml
endif

ifeq ($(strip $(MT6622_BT_SUPPORT)),true)
#PRODUCT_COPY_FILES += \
    device/rockchip/common/bluetooth/mt6622/libbluetooth_mtk.so:system/lib/libbluetooth_mtk.so \
    device/rockchip/common/bluetooth/mt6622/libbt-vendor.so:system/lib/libbt-vendor.so
endif

ifeq ($(strip $(BOARD_CONNECTIVITY_MODULE)), rtl8723as)
PRODUCT_COPY_FILES += \
    device/rockchip/common/bluetooth/realtek/bt/firmware/rtl8723as/rtl8723a_fw:system/etc/firmware/rtlbt/rtlbt_fw \
    device/rockchip/common/bluetooth/realtek/bt/firmware/rtl8723as/rtl8723a_config:system/etc/firmware/rtlbt/rtlbt_config
endif

ifeq ($(strip $(BOARD_CONNECTIVITY_MODULE)), rtl8723bs)
PRODUCT_COPY_FILES += \
    device/rockchip/common/bluetooth/realtek/bt/firmware/rtl8723bs/rtl8723b_fw:system/etc/firmware/rtlbt/rtlbt_fw

#use external or internal 32k clock
USE_EXT_32K_CLK := true
ifeq ($(strip $(USE_EXT_32K_CLK)), true)
# use external 32k clock    
PRODUCT_COPY_FILES += \
    device/rockchip/common/bluetooth/realtek/bt/firmware/rtl8723bs/rtl8723b_config.external:system/etc/firmware/rtlbt/rtlbt_config
else
# use internal 32k clock  
PRODUCT_COPY_FILES += \
    device/rockchip/common/bluetooth/realtek/bt/firmware/rtl8723bs/rtl8723b_config.internal:system/etc/firmware/rtlbt/rtlbt_config
endif
endif

ifeq ($(strip $(BOARD_CONNECTIVITY_MODULE)), rtl8723au)
PRODUCT_COPY_FILES += \
    device/rockchip/common/bluetooth/realtek/bt/firmware/rtl8723au/rtk8723a:system/etc/firmware/rtk8723a \
    device/rockchip/common/bluetooth/realtek/bt/firmware/rtl8723au/rtk8723_bt_config:system/etc/firmware/rtk8723_bt_config
endif

ifeq ($(strip $(BOARD_CONNECTIVITY_MODULE)), rtl8723bu)
PRODUCT_COPY_FILES += \
    device/rockchip/common/bluetooth/realtek/bt/firmware/rtl8723bu/rtl8723b_fw:system/etc/firmware/rtl8723b_fw \
    device/rockchip/common/bluetooth/realtek/bt/firmware/rtl8723bu/rtl8723bu_config:system/etc/firmware/rtl8723bu_config
endif

BT_FIRMWARE_FILES := $(shell ls $(CUR_PATH)/lib/firmware)
PRODUCT_COPY_FILES += \
    $(foreach file, $(BT_FIRMWARE_FILES), $(CUR_PATH)/lib/firmware/$(file):system/vendor/firmware/$(file))

include device/rockchip/common/bluetooth/console_start_bt/console_start_bt.mk

