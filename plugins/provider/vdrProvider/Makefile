#
# Makefile for a UPnP provider plugin
#
# $Id$
#

#
# This is the schema of the provider plugin. It is used
# to determine how a resource may be accessed.
#
SCHEMA = vdr

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' $(SCHEMA)Provider.cpp | awk '{ print $$6 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= gcc
CXXFLAGS ?= -g -O3 -Wall -Werror=overloaded-virtual -Wno-parentheses -fPIC

UPNPDIR ?= ../../..

INCLUDES += -I$(UPNPDIR)/include

DEFINES  += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE

OBJS     = $(SCHEMA)Provider.cpp

all: libupnp-$(SCHEMA)-provider.so

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)

libupnp-$(SCHEMA)-provider.so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(OBJS) $(LIBS) -o $@
