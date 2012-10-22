### The directory environment:

VDRDIR ?= ../../..
LIBDIR ?= ../../lib
TMPDIR ?= /tmp
UPNPDIR ?= .

### The version number of VDR's plugin API (taken from VDR's "config.h"):

VDRAPIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)
UPNPAPIVERSION = $(shell sed -ne '/define UPNPPLUGIN_VERSION/s/^.*"\(.*\)".*$$/\1/p' $(UPNPDIR)/include/plugin.h)

SUBPLUGINVERSION = 

### The C++ compiler and options:

CXX      ?= g++
ECPPC	   ?= ecppc
CXXFLAGS ?= -g -O3 -Wall -Werror=overloaded-virtual -Wno-parentheses -fPIC

### Implicit rules:

%.cpp: %.ecpp
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<
