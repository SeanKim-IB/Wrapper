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

LOCAL_CFLAGS += -D__linux__ -D__android__ -D__NBISLE__ -D__NBISLE__ -W -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-unused-but-set-variable

LOCAL_SRC_FILES  := \
	alloc.c      append.c   copy.c    date.c   \
	dec_jpeg2k.c dec_png.c  decode.c  delete.c \
	flip.c       fmtstd.c   fmttext.c getimg.c \
	globals.c    insert.c   is_an2k.c lookup.c \
	print.c      read.c     seg.c     select.c size.c \
	subst.c      to_iafis.c to_nist.c type.c   \
	type1314.c   update.c   util.c    value2.c
LOCAL_C_INCLUDES :=                              \
    $(LOCAL_PATH)/../../../include               \
    $(LOCAL_PATH)/../../../../commonnbis/include \
    $(LOCAL_PATH)/../../../../ijg/src/lib/jpegb  \
    $(LOCAL_PATH)/../../../../imgtools/include   \
    $(LOCAL_PATH)/../../../../png/src/lib/png
LOCAL_MODULE     := an2k

include $(BUILD_STATIC_LIBRARY)
