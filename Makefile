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

### dummy entry to silence the vdr >= 1.7.34 Makefile about old Makefile architecture
# PKGCFG

### this plugin has subplugins:

SUBPLUGDIR ?= ./plugins

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### The name of the shared object file:

SOFILE = libvdr-$(PLUGIN).so

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

all: $(SOFILE) subplugins i18n

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

%.cpp: %.ecpp
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) $(TNTOBJ:%.o=%.cpp) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR     = po
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmo    = $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Nmsgs  = $(addprefix $(DESTDIR)$(LOCDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.cpp)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --package-name=vdr-$(PLUGIN) --package-version=$(VERSION) --msgid-bugs-address='<see README>' -o $@ `ls $^`

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q -N $@ $<
	@touch $@

$(I18Nmsgs): $(DESTDIR)$(LOCDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	install -D -m644 $< $@

.PHONY: i18n
i18n: $(I18Nmo) $(I18Npot)

install-i18n: $(I18Nmsgs)

uninstall-i18n:
	for lang in $(shell basename --multiple --suffix=.po $(shell ls po/*.po)); do rm -v $(DESTDIR)$(LOCDIR)/$$lang/LC_MESSAGES/vdr-$(PLUGIN).mo; done

### Targets:

$(SOFILE): $(OBJS) $(TNTOBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -rdynamic -shared $(OBJS) $(TNTOBJ) $(LIBS) -o $@

install-lib: $(SOFILE)
	install -m 755 -o root -g root $(PRESTRIP) -D $^ $(DESTDIR)$(LIBDIR)/$^.$(APIVERSION)

install-resources:
	@mkdir -p $(DESTDIR)$(VDRRESDIR)
	@cp --remove-destination --recursive httpdocs $(DESTDIR)$(VDRRESDIR)

uninstall-resources:
	rm --recursive $(DESTDIR)$(VDRRESDIR)

install: install-lib install-resources install-subplugins install-i18n install-docs

uninstall: uninstall-resources uninstall-subplugins uninstall-docs uninstall-i18n
	rm --recursive $(DESTDIR)$(LIBDIR)/$(SOFILE).$(APIVERSION)

install-docs:
	@mkdir -p $(DESTDIR)$(INSDOCDIR)
	@for doc in COPYING HISTORY INSTALL README; do cp $$doc $(DESTDIR)$(INSDOCDIR); done

uninstall-docs:
	rm --recursive $(DESTDIR)$(INSDOCDIR)

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
