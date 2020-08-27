LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := ihead
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libihead.a
include $(PREBUILT_STATIC_LIBRARY)
