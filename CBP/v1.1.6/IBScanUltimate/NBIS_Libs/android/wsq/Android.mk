LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := wsq
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libwsq.a
include $(PREBUILT_STATIC_LIBRARY)
