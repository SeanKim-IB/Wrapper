LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := ioutil
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libioutil.a
include $(PREBUILT_STATIC_LIBRARY)
