$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
include device/rockchip/rk3026/BoardConfig.mk
$(call inherit-product, device/rockchip/rksdk/device.mk)
PRODUCT_BRAND := rockchip
PRODUCT_DEVICE := rk3026
PRODUCT_NAME := rk3026
PRODUCT_MANUFACTURER := rockchip

# Get the long list of APNs
PRODUCT_COPY_FILES += device/rockchip/common/phone/etc/apns-full-conf.xml:system/etc/apns-conf.xml
PRODUCT_COPY_FILES += device/rockchip/common/phone/etc/spn-conf.xml:system/etc/spn-conf.xml

PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.version = 1.0.0 \
    ro.rk.small_screen = true \
    ro.product.ota.host = www.rockchip.com:2300

#songshitian wifi only
PRODUCT_PROPERTY_OVERRIDES += \
    ro.radio.noril = true
