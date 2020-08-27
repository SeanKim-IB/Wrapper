################################################################################
# COMPILE IBScanUltimate LIBRARY
################################################################################

#
# ARCHITECTURE-DEPENDENT SETTINGS
#
ARCHDEFINES  = -D__desktop__
ARCHFLAGS    = -m32
ARCHLIBS     = Linux_x86

#
# GENERAL SETTINGS
# CROSS_COMPILE MUST HAVE BEEN DEFINED AND EXPORTED
#
CC           = gcc
STRIP        = strip
INCLUDES     = -I ../include -I . -I CaptureLib -I ThreadLib -I MatcherLib -I CxImage_Libs/include -I ../LibDeviceParallel/jni/include
WARNINGS     = -Wall
OPTIMIZATION = -O2
NBISLIBS_    = libnfiq.a libwsq.a libf2c.a libimage.a libmindtct.a \
	libbozorth3.a libfet.a libmlp.a libpca.a libcblas.a libfft.a \
	libclapck.a libihead.a libnfseg.a libpcautil.a libutil.a libjpegb.a \
	libjpegl.a libioutil.a liban2k.a
NBISLIBS     = $(patsubst %,NBIS_Libs/$(ARCHLIBS)/%,$(NBISLIBS_))
CXIMAGELIBS_    = libCxImage.a
CXIMAGELIBS     = $(patsubst %,CxImage_Libs/$(ARCHLIBS)/%,$(CXIMAGELIBS_))
LIBRARIES    = $(NBISLIBS) $(CXIMAGELIBS)
DEFINES      = -D__ppi__ -DBSD -D__linux__ -D_MULTI_THREADED -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 $(ARCHDEFINES)
FLAGSCC      = -fPIC $(ARCHFLAGS)
FLAGSAR      = -fPIC $(ARCHFLAGS) -Wl,--no-as-needed -shared -ludev -lusb -lpthread -lstdc++ -lDeviceParallel -Wl,--version-script=IBScanUltimate.vsc

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
	IBEncryption.o ParallelProc.o memwatch.o MainCapture.o MainCapture_Columbo.o \
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

