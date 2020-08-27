LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := cblas
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libcblas.a
include $(PREBUILT_STATIC_LIBRARY)
