LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := nfiq
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libnfiq.a
include $(PREBUILT_STATIC_LIBRARY)
