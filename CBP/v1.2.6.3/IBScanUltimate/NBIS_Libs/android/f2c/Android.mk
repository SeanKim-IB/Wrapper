LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := f2c
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libf2c.a
include $(PREBUILT_STATIC_LIBRARY)
