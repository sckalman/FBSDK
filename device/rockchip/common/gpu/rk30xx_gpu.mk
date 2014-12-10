ifeq ($(strip $(BOARD_USE_LCDC_COMPOSER)),true)
PRODUCT_PROPERTY_OVERRIDES += ro.sf.lcdc_composer=1
PRODUCT_COPY_FILES += \
    device/rockchip/common/gpu/libmali_smp/dedicated/libMali.so:system/lib/libMali.so \
    device/rockchip/common/gpu/libmali_smp/dedicated/libMali.so:obj/lib/libMali.so \
    device/rockchip/common/gpu/libmali_smp/dedicated/libUMP.so:system/lib/libUMP.so \
    device/rockchip/common/gpu/libmali_smp/dedicated/libUMP.so:obj/lib/libUMP.so \
    device/rockchip/common/gpu/libmali_smp/dedicated/libEGL_mali.so:system/lib/egl/libEGL_mali.so \
    device/rockchip/common/gpu/libmali_smp/dedicated/libGLESv1_CM_mali.so:system/lib/egl/libGLESv1_CM_mali.so \
    device/rockchip/common/gpu/libmali_smp/dedicated/libGLESv2_mali.so:system/lib/egl/libGLESv2_mali.so
ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk3026)
PRODUCT_COPY_FILES += \
    device/rockchip/common/gpu/libmali_smp/rk3026/mali.ko:system/lib/modules/mali.ko \
    device/rockchip/common/gpu/libmali_smp/rk3026/ump.ko:system/lib/modules/ump.ko
else
PRODUCT_COPY_FILES += \
    device/rockchip/common/gpu/libmali_smp/dedicated/mali.ko.3.0.36+:system/lib/modules/mali.ko.3.0.36+ \
    device/rockchip/common/gpu/libmali_smp/dedicated/mali.ko:system/lib/modules/mali.ko \
    device/rockchip/common/gpu/libmali_smp/dedicated/ump.ko.3.0.36+:system/lib/modules/ump.ko.3.0.36+ \
    device/rockchip/common/gpu/libmali_smp/dedicated/ump.ko:system/lib/modules/ump.ko
endif

PRODUCT_COPY_FILES += \
    device/rockchip/common/gpu/gpu_performance/dedicated/performance_info.xml:system/etc/performance_info.xml

else
PRODUCT_PROPERTY_OVERRIDES += ro.sf.lcdc_composer=0
PRODUCT_COPY_FILES += \
    device/rockchip/common/gpu/libmali_smp/libMali.so:system/lib/libMali.so \
    device/rockchip/common/gpu/libmali_smp/libMali.so:obj/lib/libMali.so \
    device/rockchip/common/gpu/libmali_smp/libUMP.so:system/lib/libUMP.so \
    device/rockchip/common/gpu/libmali_smp/libUMP.so:obj/lib/libUMP.so \
    device/rockchip/common/gpu/libmali_smp/libEGL_mali.so:system/lib/egl/libEGL_mali.so \
    device/rockchip/common/gpu/libmali_smp/libGLESv1_CM_mali.so:system/lib/egl/libGLESv1_CM_mali.so \
    device/rockchip/common/gpu/libmali_smp/libGLESv2_mali.so:system/lib/egl/libGLESv2_mali.so
ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk3026)
PRODUCT_COPY_FILES += \
    device/rockchip/common/gpu/libmali_smp/rk3026/mali.ko:system/lib/modules/mali.ko \
    device/rockchip/common/gpu/libmali_smp/rk3026/ump.ko:system/lib/modules/ump.ko \
	device/rockchip/common/gpu/libmali_smp/osmem/mali.ko.3.0.36+:system/lib/modules/mali.ko.3.0.36+ \
	device/rockchip/common/gpu/libmali_smp/osmem/ump.ko.3.0.36+:system/lib/modules/ump.ko.3.0.36+
else
PRODUCT_COPY_FILES += \
    device/rockchip/common/gpu/libmali_smp/osmem/mali.ko.3.0.36+:system/lib/modules/mali.ko.3.0.36+ \
    device/rockchip/common/gpu/libmali_smp/osmem/mali.ko:system/lib/modules/mali.ko \
    device/rockchip/common/gpu/libmali_smp/osmem/ump.ko.3.0.36+:system/lib/modules/ump.ko.3.0.36+ \
    device/rockchip/common/gpu/libmali_smp/osmem/ump.ko:system/lib/modules/ump.ko
endif

PRODUCT_COPY_FILES += \
    device/rockchip/common/gpu/gpu_performance/performance_info.xml:system/etc/performance_info.xml
endif

PRODUCT_PROPERTY_OVERRIDES += debug.hwui.render_dirty_regions=false

PRODUCT_COPY_FILES += \
    device/rockchip/common/gpu/gpu_performance/packages-compat.xml:system/etc/packages-compat.xml \
    device/rockchip/common/gpu/gpu_performance/packages-composer.xml:system/etc/packages-composer.xml \
    device/rockchip/common/gpu/gpu_performance/performance:system/bin/performance \
    device/rockchip/common/gpu/gpu_performance/libperformance_runtime.so:system/lib/libperformance_runtime.so \
    device/rockchip/common/gpu/gpu_performance/gpu.$(TARGET_BOARD_HARDWARE).so:system/lib/hw/gpu.$(TARGET_BOARD_HARDWARE).so
