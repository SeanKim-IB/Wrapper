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
#parsargs.c
LOCAL_SRC_FILES  := \
	bincopy.c  binfill.c  binpad.c   bitmasks.c \
	dilate.c   findblob.c grp4comp.c grp4deco.c \
	imageops.c img_io.c   imgavg.c   imgboost.c \
	imgdecod.c imgsnip.c  imgtype.c  imgutil.c  \
	intrlv.c   masks.c    readihdr.c \
	rgb_ycc.c  rl.c       sunrast.c  thresh.c   \
	writihdr.c
LOCAL_C_INCLUDES :=                              \
    $(LOCAL_PATH)/../../../include               \
    $(LOCAL_PATH)/../../../../an2k/include       \
    $(LOCAL_PATH)/../../../../commonnbis/include \
    $(LOCAL_PATH)/../../../../ijg/src/lib/jpegb
LOCAL_MODULE     := image

include $(BUILD_STATIC_LIBRARY)
