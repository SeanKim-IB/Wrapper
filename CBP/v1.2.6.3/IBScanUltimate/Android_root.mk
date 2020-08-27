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
#TARGET_ARCH     := arm
TARGET_PLATFORM := android-9
include $(CLEAR_VARS)

LOCAL_CFLAGS += -D__linux__ -D__embedded__ -D__android__ -D__libusb_latest__ -D_STLP_USE_NEWALLOC -fexceptions \
	-W -Wall

LOCAL_SRC_FILES  :=          \
	CaptureLib/IBEncryption.cpp         \
	CaptureLib/LinuxProc.cpp            \
	CaptureLib/IBAlgorithm.cpp          \
	CaptureLib/IBAlgorithm_Columbo.cpp  \
	CaptureLib/IBAlgorithm_Curve.cpp    \
	CaptureLib/IBAlgorithm_Holmes.cpp   \
	CaptureLib/IBAlgorithm_Sherlock.cpp \
	CaptureLib/IBAlgorithm_Watson.cpp   \
	CaptureLib/IBAlgorithm_CurveSETi.cpp   \
	CaptureLib/IBAlgorithm_Kojak.cpp   \
	CaptureLib/IBAlgorithm_Five0.cpp   \
	CaptureLib/IBAlgorithm_Enhanced.cpp   \
	CaptureLib/IBAlgorithm_SBD.cpp \
	CaptureLib/MainCapture.cpp          \
	CaptureLib/MainCapture_Columbo.cpp  \
	CaptureLib/MainCapture_Curve.cpp    \
	CaptureLib/MainCapture_Holmes.cpp   \
	CaptureLib/MainCapture_Sherlock.cpp \
	CaptureLib/MainCapture_Watson.cpp   \
	CaptureLib/MainCapture_CurveSETi.cpp   \
	CaptureLib/MainCapture_Kojak.cpp   \
	CaptureLib/MainCapture_Five0.cpp   \
	ThreadPool/Job.cpp                  \
	ThreadPool/ThreadPoolMgr.cpp        \
	ThreadPool/WorkerThread_Linux.cpp   \
	MatcherLib/IBSMAlgorithm.cpp   \
	MatcherLib/IBSMAlgorithm_ImgPro.cpp   \
	MatcherLib/IBSMAlgorithm_Match.cpp   \
	IBScanUltimateDLL.cpp
LOCAL_C_INCLUDES             := $(LOCAL_PATH)/. $(LOCAL_PATH)/../include $(LOCAL_PATH)/CaptureLib $(LOCAL_PATH)/ThreadPool $(LOCAL_PATH)/MatcherLib "$(LOCAL_PATH)/../Samples/Android/LibUsb1.0/jni/libusb" $(LOCAL_PATH)/CxImage_Libs/include
LOCAL_MODULE                 := ibscanultimate
LOCAL_LDLIBS                 := -llog  -Wl,--version-script=$(LOCAL_PATH)/IBScanUltimate.vsc
LOCAL_SHARED_LIBRARIES       += usb1.0
#LOCAL_WHOLE_STATIC_LIBRARIES += an2k cblas fet mlp ihead image ioutil jpegb jpegl mindtct nfiq pcasys png util wsq zlib
LOCAL_WHOLE_STATIC_LIBRARIES += an2k cblas fet mlp ihead image ioutil jpegb jpegl mindtct nfiq pcasys util wsq

include $(BUILD_SHARED_LIBRARY)

$(call import-module, an2k)
$(call import-module, cblas)
$(call import-module, fet)
$(call import-module, ihead)
$(call import-module, image)
$(call import-module, ioutil)
$(call import-module, jpegb)
$(call import-module, jpegl)
$(call import-module, mindtct)
$(call import-module, mlp)
$(call import-module, nfiq)
#$(call import-module, png)
$(call import-module, util)
$(call import-module, wsq)
#$(call import-module, zlib)
