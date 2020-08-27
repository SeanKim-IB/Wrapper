LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := jpegl
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libjpegl.a
include $(PREBUILT_STATIC_LIBRARY)
