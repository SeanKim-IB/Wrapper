LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := mindtct
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libmindtct.a
include $(PREBUILT_STATIC_LIBRARY)
