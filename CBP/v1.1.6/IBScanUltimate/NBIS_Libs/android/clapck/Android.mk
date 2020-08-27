LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := clapck
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libclapck.a
include $(PREBUILT_STATIC_LIBRARY)
