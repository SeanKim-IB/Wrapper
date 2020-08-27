LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE     := CxImage
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libCxImage.a
include $(PREBUILT_STATIC_LIBRARY)
