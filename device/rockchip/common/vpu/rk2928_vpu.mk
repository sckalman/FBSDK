CUR_PATH := device/rockchip/common/vpu


sf_lib_2928spec_files := $(shell ls $(CUR_PATH)/lib2928spec | grep .so)
PRODUCT_COPY_FILES += \
       $(foreach file, $(sf_lib_2928spec_files), $(CUR_PATH)/lib2928spec/$(file):system/lib/$(file))

PRODUCT_COPY_FILES += \
       $(foreach file, $(sf_lib_2928spec_files), $(CUR_PATH)/lib2928spec/$(file):obj/lib/$(file))

sf_lib_files := $(shell ls $(CUR_PATH)/lib | grep .so)
PRODUCT_COPY_FILES += \
       $(foreach file, $(sf_lib_files), $(CUR_PATH)/lib/$(file):system/lib/$(file))

PRODUCT_COPY_FILES += \
       $(foreach file, $(sf_lib_files), $(CUR_PATH)/lib/$(file):obj/lib/$(file))

PRODUCT_COPY_FILES += \
    device/rockchip/common/vpu/lib/media_codecs.xml:system/etc/media_codecs.xml \
    device/rockchip/common/vpu/lib/wfd:system/bin/wfd \
    device/rockchip/common/vpu/lib/modules/vpu_service.ko:system/lib/modules/vpu_service.ko\

PRODUCT_PACKAGES += \
	libyuvtorgb     

