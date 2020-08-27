################################################################################
# COMPILE IBScanUltimateJNI LIBRARY
################################################################################

#
# ARCHITECTURE-DEPENDENT SETTINGS
#
ARCHDEFINES = -D__embedded__
ifeq ($(ARCHABI),arm-linux-gnueabihf)
	ARCHFLAGS = -fno-exceptions -mfloat-abi=hard -mthumb -march=armv7-a -mtls-dialect=gnu
else
	ifeq ($(ARCHABI),arm-linux-gnueabi)
		ARCHFLAGS = -fno-exceptions -mfloat-abi=softfp -mthumb -march=armv7-a -mtls-dialect=gnu
	else
		ARCHFLAGS = -fno-exceptions -mfloat-abi=hard -mthumb -march=armv7-a
	endif
endif
JAVA_INCLUDE = /usr/lib/jvm/java-6-oracle/include

#
# GENERAL SETTINGS
# CROSS_COMPILE MUST HAVE BEEN DEFINED AND EXPORTED
#
CC           = $(CROSS_COMPILE)""gcc
STRIP        = $(CROSS_COMPILE)""strip
INCLUDES     = -I ../../../include -I . -I $(JAVA_INCLUDE) -I $(JAVA_INCLUDE)/linux
WARNINGS     = -Wall
OPTIMIZATION = -O3
DEFINES      = -DBSD -D__linux__ -D_MULTI_THREADED $(ARCHDEFINES)
FLAGSCC      = -fPIC $(ARCHFLAGS)
FLAGSAR      = -fPIC $(ARCHFLAGS) -Wl,--no-as-needed -shared -L ../../../lib/$(ARCHABI) -lIBScanUltimate -lpthread -lstdc++

#
# DIRECTORIES AND NAMES OF OBJECTS TO BE CREATED
#
LIBDIR = lib

OBJDIR = obj
OBJS_  = IBScan.o IBScanDevice.o
OBJS   = $(patsubst %,$(OBJDIR)/%,$(OBJS_))

#
# TOP-LEVEL RULES
#
all: $(LIBDIR)/libIBScanUltimateJNI.so

clean:
	rm -rf $(OBJDIR)
	rm -rf $(LIBDIR)

#
# RULES FOR COMPILING DIRECTORIES OF OBJECTS
#
$(OBJS): | $(OBJDIR)
$(OBJDIR):
	mkdir $(OBJDIR)
$(OBJDIR)/%.o: %.c
	$(CC) $(OPTIMIZATION) $(INCLUDES) $(WARNINGS) $(DEFINES) $(FLAGSCC) -c -o $@ $<

#
# RULES FOR CREATING LIBRARIES
#
$(LIBDIR)/libIBScanUltimateJNI.so: $(OBJS) | $(LIBDIR)
	$(CC) $(OPTIMIZATION) $(FLAGSAR) -o $@ $^
	$(STRIP) -o $@.stripped $@
$(LIBDIR):
	mkdir $(LIBDIR)

