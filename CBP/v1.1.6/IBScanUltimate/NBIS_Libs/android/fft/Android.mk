LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE     := fft
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libfft.a
include $(PREBUILT_STATIC_LIBRARY)
