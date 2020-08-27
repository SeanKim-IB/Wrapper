LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := an2k
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/liban2k.a
include $(PREBUILT_STATIC_LIBRARY)
