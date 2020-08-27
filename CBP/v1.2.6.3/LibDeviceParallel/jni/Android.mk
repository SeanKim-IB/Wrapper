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

include $(CLEAR_VARS)

#LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
	util.cpp \
	DeviceParallel.cpp

LOCAL_CFLAGS += -W -Wall -fPIC -DPIC -Wno-sign-compare

LOCAL_MODULE                 := DeviceParallel

LOCAL_C_INCLUDES             := $(LOCAL_PATH)/. $(LOCAL_PATH)/include

LOCAL_LDLIBS                 := -llog  -Wl,--version-script=$(LOCAL_PATH)/DeviceParallel.vsc

LOCAL_PRELINK_MODULE := false 

include $(BUILD_SHARED_LIBRARY)
