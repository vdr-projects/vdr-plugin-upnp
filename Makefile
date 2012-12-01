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

### our common config

-include Make.config

### Make sure that necessary options are included:

-include $(VDRDIR)/Make.global

### Allow user defined VDR options to overwrite defaults:

-include $(VDRDIR)/Make.config

SUBPLUGDIR ?= ./plugins

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### The object files (add further files here):

TNTOBJ =	httptnt/deviceDescription.o \
			httptnt/cds_scpd.o \
			httptnt/cms_scpd.o \
			httptnt/resourceStreamer.o

OBJS = 	$(PLUGIN).o \
		server/server.o \
		server/connection.o \
		server/webserver.o \
		server/service.o \
		server/connectionManager.o \
		server/contentDirectory.o \
		common/config.o \
		common/tools.o \
		common/parser.o \
		common/setup.o \
		common/ixml.o \
		media/profile.o \
		media/mediaManager.o \
		media/pluginManager.o
		
LIBS += -lupnp -lcxxtools -ltntnet -ltntdb -ldl


### The main target:

all: plugin subplugins

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

%.cpp: %.ecpp
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) $(TNTOBJ:%.o=%.cpp) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR     = po
LOCALEDIR = $(VDRDIR)/locale
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmsgs  = $(addprefix $(LOCALEDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(PLUGIN).h $(OBJS:%.o=%.cpp)  $(TNTOBJ:%.o=%.cpp)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --package-name=vdr-$(PLUGIN) --package-version=$(VERSION) --msgid-bugs-address='<see README>' -o $@ $^

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(I18Nmsgs): $(LOCALEDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: i18n
i18n: $(I18Nmsgs) $(I18Npot)

### Targets:

plugin: libvdr-$(PLUGIN).so i18n

libvdr-$(PLUGIN).so: $(OBJS) $(TNTOBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -rdynamic -shared $(OBJS) $(TNTOBJ) $(LIBS) -o $@
	@cp --remove-destination $@ $(LIBDIR)/$@.$(APIVERSION)

install: all
	@mkdir -p $(VDRPLUGINLIBDIR)
	@mkdir -p $(VDRCFGDIR)
#	@mkdir -p $(VDRRESDIR)
	install -m 755 -o root -g root $(PRESTRIP) $(LIBDIR)/libvdr-$(PLUGIN).so.$(APIVERSION) $(VDRPLUGINLIBDIR)
	cp --remove-destination --recursive httpdocs $(VDRCFGDIR)

uninstall:
	rm --recursive $(VDRPLUGINLIBDIR)/libvdr-$(PLUGIN).so.$(APIVERSION)
#	rm --recursive $(VDRCFGDIR)/httpdocs

dist: $(I18Npo) clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) --exclude debian --exclude CVS --exclude .svn --exclude .git --exclude .gitignore $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean: clean-plugin clean-subplugins

clean-plugin:
	@-rm -f $(OBJS) $(TNTOBJ) $(DEPFILE) *.so *.so.$(APIVERSION) *.tgz core* *~ $(PODIR)/*.mo $(PODIR)/*.pot

clean-subplugins:
	@for i in `ls -A -I ".*" $(SUBPLUGDIR)`; do for j in `ls -A -I ".*" $(SUBPLUGDIR)/$$i`; do $(MAKE) -f ../../../Makefile.plugins -C "$(SUBPLUGDIR)/$$i/$$j" clean; done; done

subplugins:
	@for i in `ls -A -I ".*" $(SUBPLUGDIR)`; do for j in `ls -A -I ".*" $(SUBPLUGDIR)/$$i`; do $(MAKE) -f ../../../Makefile.plugins -C "$(SUBPLUGDIR)/$$i/$$j" all || exit 1; done; done

install-subplugins:
	@for i in `ls -A -I ".*" $(SUBPLUGDIR)`; do for j in `ls -A -I ".*" $(SUBPLUGDIR)/$$i`; do $(MAKE) -f ../../../Makefile.plugins -C "$(SUBPLUGDIR)/$$i/$$j" install || exit 1; done; done

uninstall-subplugins:
	@for i in `ls -A -I ".*" $(SUBPLUGDIR)`; do for j in `ls -A -I ".*" $(SUBPLUGDIR)/$$i`; do $(MAKE) -f ../../../Makefile.plugins -C "$(SUBPLUGDIR)/$$i/$$j" uninstall; done; done
