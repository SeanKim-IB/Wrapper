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

LOCAL_SRC_FILES  :=            \
	src/lib/mindtct/binar.c    \
	src/lib/mindtct/block.c    \
	src/lib/mindtct/chaincod.c \
	src/lib/mindtct/contour.c  \
	src/lib/mindtct/detect.c   \
	src/lib/mindtct/dft.c      \
	src/lib/mindtct/free.c     \
	src/lib/mindtct/getmin.c   \
	src/lib/mindtct/globals.c  \
	src/lib/mindtct/imgutil.c  \
	src/lib/mindtct/init.c     \
	src/lib/mindtct/isempty.c  \
	src/lib/mindtct/line.c     \
	src/lib/mindtct/link.c     \
	src/lib/mindtct/log.c      \
	src/lib/mindtct/loop.c     \
	src/lib/mindtct/maps.c     \
	src/lib/mindtct/matchpat.c \
	src/lib/mindtct/minutia.c  \
	src/lib/mindtct/morph.c    \
	src/lib/mindtct/mytime.c   \
	src/lib/mindtct/quality.c  \
	src/lib/mindtct/remove.c   \
	src/lib/mindtct/results.c  \
	src/lib/mindtct/ridges.c   \
	src/lib/mindtct/shape.c    \
	src/lib/mindtct/sort.c     \
	src/lib/mindtct/to_type9.c \
	src/lib/mindtct/update.c   \
	src/lib/mindtct/util.c     \
	src/lib/mindtct/xytreps.c
LOCAL_C_INCLUDES :=                     \
    $(LOCAL_PATH)/include               \
    $(LOCAL_PATH)/../an2k/include       \
    $(LOCAL_PATH)/../commonnbis/include \
    $(LOCAL_PATH)/../imgtools/include
LOCAL_MODULE     := mindtct

include $(BUILD_STATIC_LIBRARY)
