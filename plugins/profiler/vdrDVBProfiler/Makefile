#
# Makefile for a UPnP profiler plugin
#
# $Id$
#

#
# This is the profile of the profiler plugin. It is used
# to determine how a resource may be accessed.
#
PROFILE = dvb

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' $(PROFILE)Profiler.cpp | awk '{ print $$6 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= g++
CXXFLAGS ?= -g -O3 -Wall -Werror=overloaded-virtual -Wno-parentheses -fPIC

UPNPDIR ?= ../../..
VDRLIBDIR ?= $(UPNPDIR)/../../lib

APIVERSION = $(shell sed -ne '/define UPNPPLUGIN_VERSION/s/^.*"\(.*\)".*$$/\1/p' $(UPNPDIR)/include/plugin.h)

VDRDIR ?= $(UPNPDIR)/../../..
VDRAPIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)

INCLUDES += -I$(UPNPDIR)/include -I$(VDRDIR)/include

DEFINES  += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

OBJS     = $(PROFILE)Profiler.o

LIBS	 = -L$(VDRLIBDIR) -Wl,-R$(VDRLIBDIR) $(VDRLIBDIR)/libvdr-upnp.so.$(VDRAPIVERSION)

all: libupnp-$(PROFILE)-profiler.so

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)

libupnp-$(PROFILE)-profiler.so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(OBJS) $(LIBS) -o $@
	@cp --remove-destination $@ $(VDRLIBDIR)/$@.$(APIVERSION)
	
clean:
	@-rm -f $(OBJS) $(DEPFILE) *.so *.so.$(APIVERSION) core* *~
