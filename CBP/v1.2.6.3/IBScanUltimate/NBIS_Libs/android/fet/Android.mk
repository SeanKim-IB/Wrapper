LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := fet
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libfet.a
include $(PREBUILT_STATIC_LIBRARY)
