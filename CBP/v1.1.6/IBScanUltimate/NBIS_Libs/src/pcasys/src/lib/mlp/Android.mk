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
	accum.c    acs.c      acsmaps.c  boltz.c    \
	ch_bprms.c csopiwh.c  cvr.c      cwrite.c   \
	e_and_g.c  eb_cat_e.c eb_cat_w.c eb.c       \
	ef.c       endopt.c   fsaso.c    get_phr.c  \
	getpat.c   got_blk.c  got_c.c    got_nc_c.c \
	is_we.c    lbfgs_dr.c lbfgs.c    lgl_pnm.c  \
	lgl_tbl.c  little.c   lng2shrt.c mlpcla.c   \
	mtch_pnm.c neverset.c optchk.c   optwts.c   \
	pat_io.c   rd_cwts.c  rd_words.c rprt_prs.c \
	runmlp.c   scanspec.c scg.c      set_fpw.c  \
	st_nv_ok.c strm_fmt.c target.c   tsp_w.c    \
	uni.c      wts.c
LOCAL_C_INCLUDES :=                \
    $(LOCAL_PATH)/../../../include \
    $(LOCAL_PATH)/../../../../commonnbis/include \
    $(LOCAL_PATH)/../../../../imgtools/include
LOCAL_MODULE     := mlp

include $(BUILD_STATIC_LIBRARY)
