LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := mlp
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libmlp.a
include $(PREBUILT_STATIC_LIBRARY)
