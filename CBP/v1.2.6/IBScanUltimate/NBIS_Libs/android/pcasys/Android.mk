LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := pcasys
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libpcasys.a
include $(PREBUILT_STATIC_LIBRARY)
