ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali400)
LOCAL_PATH := $(call my-dir)

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3026)
ifeq ($(strip $(TARGET_PRODUCT)),rk3026)

    include $(CLEAR_VARS)
    LOCAL_PREBUILT_LIBS := hwcomposer.rk30board.so
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
    include $(BUILD_MULTI_PREBUILT)

    include $(CLEAR_VARS)
    LOCAL_PREBUILT_LIBS := gralloc.rk30board.so
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
    include $(BUILD_MULTI_PREBUILT)

endif
endif

endif
