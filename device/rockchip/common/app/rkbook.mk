CUR_PATH := device/rockchip/common/app
PRODUCT_PACKAGES += \
    BooksProvider \
    RKEBookReader
PRODUCT_COPY_FILES += \
    $(CUR_PATH)/rkbook/bin/adobedevchk:system/bin/adobedevchk \
    $(CUR_PATH)/rkbook/lib/libadobe_rmsdk.so:system/lib/libadobe_rmsdk.so \
    $(CUR_PATH)/rkbook/lib/libRkDeflatingDecompressor.so:system/lib/libRkDeflatingDecompressor.so \
    $(CUR_PATH)/rkbook/lib/librm_ssl.so:system/lib/librm_ssl.so \
    $(CUR_PATH)/rkbook/lib/libflip.so:system/lib/libflip.so \
    $(CUR_PATH)/rkbook/lib/librm_crypto.so:system/lib/librm_crypto.so \
    $(CUR_PATH)/rkbook/lib/rmsdk.ver:system/lib/rmsdk.ver \
    $(CUR_PATH)/rkbook/fonts/adobefonts/AdobeMyungjoStd.bin:system/fonts/adobefonts/AdobeMyungjoStd.bin \
    $(CUR_PATH)/rkbook/fonts/adobefonts/CRengine.ttf:system/fonts/adobefonts/CRengine.ttf \
    $(CUR_PATH)/rkbook/fonts/adobefonts/RyoGothicPlusN.bin:system/fonts/adobefonts/RyoGothicPlusN.bin \
    $(CUR_PATH)/rkbook/fonts/adobefonts/AdobeHeitiStd.bin:system/fonts/adobefonts/AdobeHeitiStd.bin \
    $(CUR_PATH)/rkbook/fonts/adobefonts/AdobeMingStd.bin:system/fonts/adobefonts/AdobeMingStd.bin
