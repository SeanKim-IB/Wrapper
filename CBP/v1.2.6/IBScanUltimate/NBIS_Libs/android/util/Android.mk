LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := util
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libutil.a
include $(PREBUILT_STATIC_LIBRARY)
