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
# lsame.c
LOCAL_SRC_FILES  := \
	ilaenv.c slacpy.c slae2.c  \
	slaebz.c slaev2.c slagtf.c slagts.c \
	slamc1.c slamc2.c slamc3.c slamc4.c \
	slamc5.c slamch.c slanst.c slansy.c \
	slapy2.c slarf.c  slarfb.c slarfg.c \
	slarft.c slarnv.c slartg.c slaruv.c \
	slascl.c slaset.c slasr.c  slasrt.c \
	slassq.c slatrd.c sorg2l.c sorg2r.c \
	sorgql.c sorgqr.c sorgtr.c sorm2l.c \
	sorm2r.c sormql.c sormqr.c sormtr.c \
	sstebz.c sstein.c ssteqr.c ssterf.c \
	ssyevx.c ssytd2.c ssytrd.c
LOCAL_C_INCLUDES :=                \
    $(LOCAL_PATH)/../../../include
LOCAL_MODULE     := clapck

include $(BUILD_STATIC_LIBRARY)
