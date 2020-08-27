################################################################################
# COMPILE IBScanUltimate TEST APPLICATION
################################################################################

#
# ARCHITECTURE-DEPENDENT SETTINGS
#
ifeq ($(ARCHABI),arm-linux-gnueabihf)
	ARCHFLAGS = -mfloat-abi=hard -march=armv6 -marm 
	CROSS_COMPILE_LIB_PATH = ~/Toolchain/Linaro/gcc-linaro-arm-linux-gnueabihf-4.8-2013.05_linux/lib
else
	ifeq ($(ARCHABI),arm-linux-gnueabi)
		ARCHFLAGS = -mfloat-abi=soft -mthumb -march=armv6
	  CROSS_COMPILE_LIB_PATH = ~/Toolchain/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/arm-none-linux-gnueabi/libc/usr/lib
	else
		ARCHFLAGS = -mfloat-abi=hard -mfpu=vfp -march=armv6
		CROSS_COMPILE_LIB_PATH = ~/Toolchain/Linaro/raspberry/gcc-4.7-linaro-rpi-gnueabihf/lib
	endif
endif

#
# GENERAL SETTINGS
# CROSS_COMPILE MUST HAVE BEEN DEFINED AND EXPORTED
#
CC           = $(CROSS_COMPILE)""gcc
INCLUDES     = -I ../../../include
WARNINGS     = -Wall
OPTIMIZATION = -O2
LIBRARIES    = -L$(CROSS_COMPILE_LIB_PATH) -lusb -lstdc++ -lpthread -L ../../../lib/$(ARCHABI) -l IBScanUltimate 
DEFINES      = -DBSD -D__linux__ -D_MULTI_THREADED $(ARCHDEFINES)
FLAGSCC      = -fPIC $(ARCHFLAGS)
FLAGSAR      = -Wl,--no-as-needed $(ARCHFLAGS)

#
# DIRECTORIES AND NAMES OF OBJECTS TO BE CREATED
#
BINDIR = bin

OBJDIR = obj
OBJS_  = testScanU.o
OBJS   = $(patsubst %,$(OBJDIR)/%,$(OBJS_))

#
# TOP-LEVEL RULES
#
all: $(BINDIR)/testScanU

clean:
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)

#
# RULES FOR COMPILING DIRECTORIES OF OBJECTS
#
$(OBJS): | $(OBJDIR)
$(OBJDIR):
	mkdir $(OBJDIR)
$(OBJDIR)/%.o: %.cpp
	$(CC) $(OPTIMIZATION) $(INCLUDES) $(WARNINGS) $(DEFINES) $(FLAGSCC) -c -o $@ $<

#
# RULES FOR CREATING BINARIES
#
$(BINDIR)/testScanU: $(OBJS) | $(BINDIR)
	$(CC) $(OPTIMIZATION) $(FLAGSAR) $(LIBRARIES) -o $@ $^
$(BINDIR):
	mkdir $(BINDIR)

