#
# Makefile for a Video Disk Recorder plugin
#
# $Id$

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
# IMPORTANT: the presence of this macro is important for the Make.config
# file. So it must be defined, even if it is not used here!
#
PLUGIN = upnp
COMMON = common.h

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' $(PLUGIN).cpp | awk '{ print $$6 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= g++
CXXFLAGS ?= -fPIC -g -Wall -O2 -Wextra -Woverloaded-virtual -Wno-parentheses -Wl,-R/usr/local/lib

### The directory environment:

VDRDIR = ../../..
LIBDIR = ../../lib
TMPDIR = /tmp

WEBDIR = $(shell grep '\#define UPNP_DIR_PRESENTATION*' $(COMMON) | awk '{ print $$3 }' | sed -e 's/["/]//g')

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

#DESDIR = /var/lib/vdrdevel/plugins/$(PLUGIN)
DESDIR = $(CONFDIR)/plugins/$(PLUGIN)

### The version number of VDR's plugin API (taken from VDR's "config.h"):

APIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

LIBS += -lupnp -lixml -lsqlite3

INCLUDES += -I$(VDRDIR)/include	-I/usr/include \

DEFINES += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"'

### The object files (add further files here):

# Root folder
OBJS = $(PLUGIN).o \
		common.o \
		misc/menusetup.o \
		misc/util.o \
		misc/config.o \
		misc/search.o \
		database/database.o \
		database/metadata.o \
		database/object.o \
		database/resources.o \
		server/server.o \
		upnpcomponents/dlna.o \
		upnpcomponents/upnpwebserver.o \
		upnpcomponents/upnpservice.o \
		upnpcomponents/connectionmanager.o \
		upnpcomponents/contentdirectory.o \
		receiver/livereceiver.o \
		receiver/recplayer.o \
		receiver/filehandle.o \

### The main target:

all: libvdr-$(PLUGIN).so i18n

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $(LIBS) -o $@ $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies

$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR     = po
LOCALEDIR = $(VDRDIR)/locale
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmsgs  = $(addprefix $(LOCALEDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.cpp)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --msgid-bugs-address='<see README>' -o $@ $^

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(I18Nmsgs): $(LOCALEDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: i18n
i18n: $(I18Nmsgs) $(I18Npot)

### Targets:

libvdr-$(PLUGIN).so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LIBS) -shared $(OBJS) -o $@ -lc
	@cp --remove-destination $@ $(LIBDIR)/$@.$(APIVERSION)
	@-rm -rf $(DESDIR)/$(WEBDIR)
	@mkdir -p $(DESDIR)/$(WEBDIR)
	@cp --remove-destination -r $(WEBDIR)/* $(DESDIR)/$(WEBDIR)

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(OBJS) $(DEPFILE) *.so *.tgz core* *~ $(PODIR)/*.mo $(PODIR)/*.pot
