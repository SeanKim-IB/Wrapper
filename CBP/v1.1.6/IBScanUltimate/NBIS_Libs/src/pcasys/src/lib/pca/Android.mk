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

LOCAL_SRC_FILES  := \
	combine.c eigen.c enhnc.c    gr_cm.c    \
	grphcs.c  inits.c mlp_sing.c pnn.c      \
	pseudo.c  r92.c   r92a.c     readfing.c \
	results.c ridge.c sgmnt.c    trnsfrm.c
LOCAL_C_INCLUDES :=                \
    $(LOCAL_PATH)/../../../include \
    $(LOCAL_PATH)/../../../../commonnbis/include \
    $(LOCAL_PATH)/../../../../imgtools/include
LOCAL_MODULE     := pca

include $(BUILD_STATIC_LIBRARY)
