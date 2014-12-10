# config.mk
# 
# Product-specific compile-time definitions.
#
TARGET_BOARD_PLATFORM := rk3026
TARGET_BOARD_PLATFORM_GPU := mali400
BOARD_USE_LOW_MEM := true
PRODUCT_PACKAGE_OVERLAYS += device/rockchip/rk3026/overlay
include device/rockchip/rksdk/BoardConfig.mk
