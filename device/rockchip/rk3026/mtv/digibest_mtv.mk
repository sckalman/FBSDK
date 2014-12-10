# android 4.4.2 didn't need mtvmfservice,libstagefright_soft_aacdec.so and libCdAbLayer.so,so delete them ,by sakura

PRODUCT_PACKAGES += \
	TV1segPositivo
	
#PRODUCT_COPY_FILES += \
	device/rockchip/rk3026/mtv/jellyservice:system/bin/jellyservice \
#	device/rockchip/rk3026/mtv/mtvmfservice:system/bin/mtvmfservice \
#	device/rockchip/rk3026/mtv/libCdAbLayer.so:system/lib/libCdAbLayer.so \
	device/rockchip/rk3026/mtv/libJellyPG.so:system/lib/libJellyPG.so \
	device/rockchip/rk3026/mtv/libMTVMFCls.so:system/lib/libMTVMFCls.so \
#	device/rockchip/rk3026/mtv/libstagefright_soft_aacdec.so:system/lib/libstagefright_soft_aacdec.so \


PRODUCT_COPY_FILES += \
                      device/rockchip/rk3026/mtv/jellyservice:system/bin/jellyservice \
                      device/rockchip/rk3026/mtv/libJellyPG.so:system/lib/libJellyPG.so \
                      device/rockchip/rk3026/mtv/libMTVMFCls.so:system/lib/libMTVMFCls.so 

