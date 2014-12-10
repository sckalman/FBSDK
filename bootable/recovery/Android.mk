# Copyright (C) 2007 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    recovery.cpp \
    bootloader.cpp \
    install.cpp \
    roots.cpp \
    ui.cpp \
    screen_ui.cpp \
    verifier.cpp \
    adb_install.cpp \
    rkimage.cpp

LOCAL_MODULE := recovery

LOCAL_FORCE_STATIC_EXECUTABLE := true

RECOVERY_API_VERSION := 3
RECOVERY_FSTAB_VERSION := 2
LOCAL_CFLAGS += -DRECOVERY_API_VERSION=$(RECOVERY_API_VERSION)
LOCAL_CFLAGS += -D_FILE_OFFSET_BITS=64

LOCAL_STATIC_LIBRARIES := \
    libext4_utils_static \
    libsparse_static \
    libminzip \
    libz \
    libmtdutils \
    libmincrypt \
    libminadbd \
    libminui \
    libpixelflinger_static \
    libpng \
    libfs_mgr \
    libcutils \
    liblog \
    libselinux \
    libstdc++ \
    libm \
    libc \
    libedify \
    libapplypatch \
    libminelf \
    librsa \
    libcrc32 \
    librk_emmcutils  

ifeq ($(TARGET_USERIMAGES_USE_EXT4), true)
    LOCAL_CFLAGS += -DUSE_EXT4
    LOCAL_C_INCLUDES += system/extras/ext4_utils
    LOCAL_STATIC_LIBRARIES += libext4_utils_static libz
endif

ifeq ($(RECOVERY_UPDATEIMG_RSA_CHECK), true)
$(warning recovery use updateimg rsa check!)
LOCAL_CFLAGS += -DUSE_RSA_CHECK
else
$(warning recovery use updateimg crc32 check!)
endif

ifeq ($(RECOVERY_BOARD_ID), true)
$(warning recovery use board id!)
LOCAL_CFLAGS += -DUSE_BOARD_ID
LOCAL_STATIC_LIBRARIES += libboard_id_recovery libxml2_recovery
else
$(warning recovery not use board id!)
endif

ifeq ($(strip $(TARGET_BOARD_HARDWARE)),rk30board)
LOCAL_CFLAGS += -DTARGET_RK30
endif
ifeq ($(strip $(TARGET_BOARD_HARDWARE)),rk2928board)
LOCAL_CFLAGS += -DTARGET_RK30
endif

# This binary is in the recovery ramdisk, which is otherwise a copy of root.
# It gets copied there in config/Makefile.  LOCAL_MODULE_TAGS suppresses
# a (redundant) copy of the binary in /system/bin for user builds.
# TODO: Build the ramdisk image in a more principled way.
LOCAL_MODULE_TAGS := eng

ifeq ($(TARGET_RECOVERY_UI_LIB),)
  LOCAL_SRC_FILES += default_device.cpp
else
  LOCAL_STATIC_LIBRARIES += $(TARGET_RECOVERY_UI_LIB)
endif

# TARGET_BOARD_PLATFORM is change from rockchip to rk29xx or rk30xx
# so force TARGET_BOARD_PLATFORM to be rockchip in recovery cpp file
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM=rockchip
LOCAL_CFLAGS += -fpermissive

LOCAL_C_INCLUDES += system/extras/ext4_utils

include $(BUILD_EXECUTABLE)



include $(CLEAR_VARS)
LOCAL_MODULE := verifier_test
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_TAGS := tests
LOCAL_SRC_FILES := \
    verifier_test.cpp \
    verifier.cpp \
    ui.cpp
LOCAL_STATIC_LIBRARIES := \
    libmincrypt \
    libminui \
    libcutils \
    libstdc++ \
    libc
include $(BUILD_EXECUTABLE)


include $(LOCAL_PATH)/minui/Android.mk \
    $(LOCAL_PATH)/minelf/Android.mk \
    $(LOCAL_PATH)/minzip/Android.mk \
    $(LOCAL_PATH)/minadbd/Android.mk \
    $(LOCAL_PATH)/mtdutils/Android.mk \
    $(LOCAL_PATH)/tools/Android.mk \
    $(LOCAL_PATH)/edify/Android.mk \
    $(LOCAL_PATH)/updater/Android.mk \
    $(LOCAL_PATH)/emmcutils/Android.mk	\
    $(LOCAL_PATH)/applypatch/Android.mk \
    $(LOCAL_PATH)/rsa/Android.mk	\
    $(LOCAL_PATH)/crc/Android.mk	\
    $(LOCAL_PATH)/board_id/Android.mk	\
    $(LOCAL_PATH)/libxml2/Android.mk
    
