LOCAL_PATH := $(call my-dir)

ifeq ($(strip $(BOARD_CONNECTIVITY_MODULE)), rda587x)

include $(CLEAR_VARS)
BDROID_DIR := $(TOP_DIR)external/bluetooth/bluedroid

LOCAL_SRC_FILES := \
  rda.c \
  rda_hal_config.c

LOCAL_C_INCLUDES := \
    $(BDROID_DIR)/hci/include


LOCAL_MODULE := libbluetooth_rda
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  bt_drv.c

LOCAL_C_INCLUDES := \
    $(BDROID_DIR)/hci/include
LOCAL_CFLAGS := 

LOCAL_MODULE := libbt-vendor
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libbluetooth_rda
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
include $(BUILD_SHARED_LIBRARY)
endif
