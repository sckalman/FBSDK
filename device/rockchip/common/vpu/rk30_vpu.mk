CUR_PATH := device/rockchip/common/vpu
sf_lib_files := $(shell ls $(CUR_PATH)/lib | grep .so)
PRODUCT_COPY_FILES += \
       $(foreach file, $(sf_lib_files), $(CUR_PATH)/lib/$(file):system/lib/$(file))

PRODUCT_COPY_FILES += \
       $(foreach file, $(sf_lib_files), $(CUR_PATH)/lib/$(file):obj/lib/$(file))

PRODUCT_COPY_FILES += \
    device/rockchip/common/vpu/lib/media_codecs.xml:system/etc/media_codecs.xml \
    device/rockchip/common/vpu/lib/modules_smp/vpu_service.ko.3.0.36+:system/lib/modules/vpu_service.ko.3.0.36+ \
    device/rockchip/common/vpu/lib/modules_smp/vpu_service.ko:system/lib/modules/vpu_service.ko\
    device/rockchip/common/vpu/lib/modules_smp/rk30_mirroring.ko.3.0.8+:system/lib/modules/rk30_mirroring.ko.3.0.8+\
    device/rockchip/common/vpu/lib/modules_smp/rk30_mirroring.ko.3.0.36+:system/lib/modules/rk30_mirroring.ko.3.0.36+ 

PRODUCT_PACKAGES += \
	libyuvtorgb     

