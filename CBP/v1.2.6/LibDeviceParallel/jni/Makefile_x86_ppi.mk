################################################################################
# COMPILE DeviceParallel LIBRARY
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
INCLUDES     = -I ./include
WARNINGS     = -Wall
OPTIMIZATION = -O2
DEFINES      = -DBSD -D__linux__ -D_MULTI_THREADED -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 $(ARCHDEFINES)
FLAGSCC      = -fPIC $(ARCHFLAGS)
FLAGSAR      = -fPIC $(ARCHFLAGS) -Wl,--no-as-needed -shared -lpthread -lstdc++ -Wl,--version-script=DeviceParallel.vsc


#
# DIRECTORIES AND NAMES OF OBJECTS TO BE CREATED
#
LIBDIR = lib

OBJDIR = obj
OBJS_  = DeviceParallel.o
OBJS   = $(patsubst %,$(OBJDIR)/%,$(OBJS_))


#
# TOP-LEVEL RULES
#
all: $(LIBDIR)/libDeviceParallel.so

clean:
	rm -rf $(OBJDIR)
	rm -rf $(LIBDIR)

install:
	sudo cp $(LIBDIR)/libDeviceParallel.so /usr/lib/

#
# RULES FOR COMPILING DIRECTORIES OF OBJECTS
#
$(OBJS): | $(OBJDIR)
$(OBJDIR):
	mkdir $(OBJDIR)
$(OBJDIR)/%.o: %.cpp
	$(CC) $(OPTIMIZATION) $(INCLUDES) $(WARNINGS) $(DEFINES) $(FLAGSCC) -c -o $@ $<

#
# RULES FOR CREATING LIBRARIES
#
$(LIBDIR)/libDeviceParallel.so: $(OBJS) | $(LIBDIR)
	$(CC) $(OPTIMIZATION) $(FLAGSAR) -o $@ $^ 
	$(STRIP) -o $@.stripped $@
$(LIBDIR):
	mkdir $(LIBDIR)
