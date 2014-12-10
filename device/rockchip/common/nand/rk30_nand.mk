ifeq ($(strip $(NAND_UPDATE)),true)
   PRODUCT_COPY_FILES += \
	device/rockchip/common/nand/lib/update/rk30xxnand_ko.ko.3.0.36+:root/rk30xxnand_ko.ko.3.0.36+ \
	device/rockchip/common/nand/lib/update/rk30xxnand_ko.ko.3.0.8+:root/rk30xxnand_ko.ko.3.0.8+ \
    device/rockchip/common/nand/lib/drmboot.ko:root/drmboot.ko
else
   PRODUCT_COPY_FILES += \
    device/rockchip/common/nand/lib/rk30xxnand_ko.ko.3.0.36+:root/rk30xxnand_ko.ko.3.0.36+ \
	device/rockchip/common/nand/lib/rk30xxnand_ko.ko.3.0.8+:root/rk30xxnand_ko.ko.3.0.8+ \
	device/rockchip/common/nand/lib/drmboot.ko:root/drmboot.ko
endif
