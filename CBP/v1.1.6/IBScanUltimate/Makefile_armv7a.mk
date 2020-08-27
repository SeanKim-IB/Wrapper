################################################################################
# COMPILE IBScanUltimate LIBRARY
################################################################################

#
# ARCHITECTURE-DEPENDENT SETTINGS
#
ARCHDEFINES = -D__embedded__
ifeq ($(ARCHABI),arm-linux-gnueabihf)
	ARCHFLAGS = -fexceptions -mfloat-abi=hard -mthumb -march=armv7-a -mtls-dialect=gnu
	CROSS_COMPILE_LIB_PATH = ~/Toolchain/Linaro/gcc-linaro-arm-linux-gnueabihf-4.8-2013.05_linux/lib
else
	ifeq ($(ARCHABI),arm-linux-gnueabi)
		ARCHFLAGS = -fexceptions -mfloat-abi=softfp -mthumb -march=armv7-a -mtls-dialect=gnu
	  CROSS_COMPILE_LIB_PATH = ~/Toolchain/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/arm-none-linux-gnueabi/libc/usr/lib
	else
		ARCHFLAGS = -fexceptions -mfloat-abi=hard -mthumb -march=armv7-a
	  CROSS_COMPILE_LIB_PATH = ~/Toolchain/Atmel-ATSAMA5D3-BuildRoot/buildroot-at91/output/host/usr/lib
	endif
endif
ARCHLIBS = Linux_armv7a

#
# GENERAL SETTINGS
# CROSS_COMPILE MUST HAVE BEEN DEFINED AND EXPORTED
#
CC           = $(CROSS_COMPILE)""gcc
STRIP        = $(CROSS_COMPILE)""strip
INCLUDES     = -I ../include -I . -I CaptureLib -I ThreadLib -I MatcherLib -I CxImage_Libs/include
WARNINGS     = -Wall
OPTIMIZATION = -O3
NBISLIBS_    = libnfiq.a libwsq.a libf2c.a libimage.a libmindtct.a \
	libbozorth3.a libfet.a libmlp.a libpca.a libcblas.a libfft.a \
	libclapck.a libihead.a libnfseg.a libpcautil.a libutil.a libjpegb.a \
	libjpegl.a libioutil.a liban2k.a
NBISLIBS     = $(patsubst %,NBIS_Libs/$(ARCHLIBS)/$(ARCHABI)/%,$(NBISLIBS_))
CXIMAGELIBS_    = libCxImage.a
CXIMAGELIBS     = $(patsubst %,CxImage_Libs/$(ARCHLIBS)/$(ARCHABI)/%,$(CXIMAGELIBS_))
LIBRARIES    = $(NBISLIBS) $(CXIMAGELIBS)
DEFINES      = -DBSD -D__linux__ -D_MULTI_THREADED $(ARCHDEFINES)
FLAGSCC      = -fPIC $(ARCHFLAGS)
#FLAGSAR      = -Wl,--no-as-needed -shared -Wl,--version-script=IBScanUltimate.vsc $(ARCHFLAGS)
FLAGSAR      = -fPIC $(ARCHFLAGS) -Wl,--no-as-needed -shared -L$(CROSS_COMPILE_LIB_PATH) -lusb -lpthread -lstdc++ -Wl,--version-script=IBScanUltimate.vsc

#
# DIRECTORIES AND NAMES OF OBJECTS TO BE CREATED
#
LIBDIR = lib

OBJDIR = obj
OBJS_  = IBScanUltimateDLL.o
OBJS   = $(patsubst %,$(OBJDIR)/%,$(OBJS_))

CAPTUREOBJDIR = obj/CaptureLib
CAPTUREOBJS_  = IBAlgorithm.o IBAlgorithm_Columbo.o IBAlgorithm_Curve.o \
	IBAlgorithm_Holmes.o IBAlgorithm_Sherlock.o IBAlgorithm_Watson.o \
	IBAlgorithm_CurveSETi.o IBAlgorithm_Kojak.o IBAlgorithm_Enhanced.o \
	IBAlgorithm_SBD.o IBAlgorithm_Five0.o \
	IBEncryption.o LinuxProc.o memwatch.o MainCapture.o MainCapture_Columbo.o \
	MainCapture_Curve.o MainCapture_Holmes.o MainCapture_Sherlock.o \
	MainCapture_Watson.o MainCapture_CurveSETi.o MainCapture_Kojak.o \
	MainCapture_Five0.o
CAPTUREOBJS   = $(patsubst %,$(CAPTUREOBJDIR)/%,$(CAPTUREOBJS_))

THREADOBJDIR = obj/ThreadPool
THREADOBJS_  = Job.o WorkerThread_Linux.o ThreadPoolMgr.o
THREADOBJS   = $(patsubst %,$(THREADOBJDIR)/%,$(THREADOBJS_))

MATCHEROBJDIR = obj/MatcherLib
MATCHEROBJS_  = IBSMAlgorithm.o IBSMAlgorithm_ImgPro.o IBSMAlgorithm_Match.o
MATCHEROBJS   = $(patsubst %,$(MATCHEROBJDIR)/%,$(MATCHEROBJS_))

#
# TOP-LEVEL RULES
#
all: $(LIBDIR)/libIBScanUltimate.so

clean:
	rm -rf $(OBJDIR)
	rm -rf $(LIBDIR)

#
# RULES FOR COMPILING DIRECTORIES OF OBJECTS
#
$(OBJS): | $(OBJDIR)
$(OBJDIR):
	mkdir $(OBJDIR)
$(OBJDIR)/%.o: %.cpp
	$(CC) $(OPTIMIZATION) $(INCLUDES) $(WARNINGS) $(DEFINES) $(FLAGSCC) -c -o $@ $<

$(CAPTUREOBJS): | $(CAPTUREOBJDIR)
$(CAPTUREOBJDIR):
	mkdir $(CAPTUREOBJDIR)
$(CAPTUREOBJDIR)/%.o: CaptureLib/%.cpp
	$(CC) $(OPTIMIZATION) $(INCLUDES) $(WARNINGS) $(DEFINES) $(FLAGSCC) -c -o $@ $<
$(CAPTUREOBJDIR)/%.o: CaptureLib/%.c
	$(CC) $(OPTIMIZATION) $(INCLUDES) $(WARNINGS) $(DEFINES) $(FLAGSCC) -c -o $@ $<

$(THREADOBJS): | $(THREADOBJDIR)
$(THREADOBJDIR):
	mkdir $(THREADOBJDIR)
$(THREADOBJDIR)/%.o: ThreadPool/%.cpp
	$(CC) $(OPTIMIZATION) $(INCLUDES) $(WARNINGS) $(DEFINES) $(FLAGSCC) -c -o $@ $<

$(MATCHEROBJS): | $(MATCHEROBJDIR)
$(MATCHEROBJDIR):
	mkdir $(MATCHEROBJDIR)
$(MATCHEROBJDIR)/%.o: MatcherLib/%.cpp
	$(CC) $(OPTIMIZATION) $(INCLUDES) $(WARNINGS) $(DEFINES) $(FLAGSCC) -c -o $@ $<

#
# RULES FOR CREATING LIBRARIES
#
$(LIBDIR)/libIBScanUltimate.so: $(OBJS) $(CAPTUREOBJS) $(THREADOBJS) $(MATCHEROBJS) | $(LIBDIR)
	$(CC) $(OPTIMIZATION) $(FLAGSAR) -o $@ $^ $(LIBRARIES)
	$(STRIP) -o $@.stripped $@
$(LIBDIR):
	mkdir $(LIBDIR)

