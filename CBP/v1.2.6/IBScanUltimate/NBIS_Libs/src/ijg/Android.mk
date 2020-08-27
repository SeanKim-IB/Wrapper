# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH      := $(call my-dir)
TARGET_ARCH     := arm
TARGET_PLATFORM := android-9
include $(CLEAR_VARS)

LOCAL_CFLAGS += -D__linux__ -D__android__ -D__NBISLE__ -W -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-unused-but-set-variable

LOCAL_SRC_FILES  :=        \
    src/lib/jpegb/jcapimin.c \
    src/lib/jpegb/jcapistd.c \
    src/lib/jpegb/jccoefct.c \
    src/lib/jpegb/jccolor.c  \
    src/lib/jpegb/jcdctmgr.c \
    src/lib/jpegb/jchuff.c   \
    src/lib/jpegb/jcinit.c   \
    src/lib/jpegb/jcmainct.c \
    src/lib/jpegb/jcmarker.c \
    src/lib/jpegb/jcmaster.c \
    src/lib/jpegb/jcomapi.c  \
    src/lib/jpegb/jcparam.c  \
    src/lib/jpegb/jcphuff.c  \
    src/lib/jpegb/jcprepct.c \
    src/lib/jpegb/jcsample.c \
    src/lib/jpegb/jctrans.c  \
    src/lib/jpegb/jdapimin.c \
    src/lib/jpegb/jdapistd.c \
    src/lib/jpegb/jdatadst.c \
    src/lib/jpegb/jdatasrc.c \
    src/lib/jpegb/jdcoefct.c \
    src/lib/jpegb/jdcolor.c  \
    src/lib/jpegb/jddctmgr.c \
    src/lib/jpegb/jdhuff.c   \
    src/lib/jpegb/jdinput.c  \
    src/lib/jpegb/jdmainct.c \
    src/lib/jpegb/jdmarker.c \
    src/lib/jpegb/jdmaster.c \
    src/lib/jpegb/jdmerge.c  \
    src/lib/jpegb/jdphuff.c  \
    src/lib/jpegb/jdpostct.c \
    src/lib/jpegb/jdsample.c \
    src/lib/jpegb/jdtrans.c  \
    src/lib/jpegb/jerror.c   \
    src/lib/jpegb/jfdctflt.c \
    src/lib/jpegb/jfdctfst.c \
    src/lib/jpegb/jfdctint.c \
    src/lib/jpegb/jidctflt.c \
    src/lib/jpegb/jidctfst.c \
    src/lib/jpegb/jidctint.c \
    src/lib/jpegb/jidctred.c \
    src/lib/jpegb/jquant1.c  \
    src/lib/jpegb/jquant2.c  \
    src/lib/jpegb/jutils.c   \
    src/lib/jpegb/jmemmgr.c  \
	src/lib/jpegb/membuf.c   \
	src/lib/jpegb/decoder.c  \
	src/lib/jpegb/encoder.c  \
	src/lib/jpegb/ppi.c      \
	src/lib/jpegb/marker.c   \
	src/lib/jpegb/jmemansi.c
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/src/lib/jpegb \
    $(LOCAL_PATH)/../imgtools/include \
    $(LOCAL_PATH)/../commonnbis/include
LOCAL_MODULE     := jpegb

include $(BUILD_STATIC_LIBRARY)
