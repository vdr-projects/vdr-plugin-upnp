#
# Makefile for a UPnP subplugin
#
# $Id$
#

SUBPLUGIN = dvb
CATEGORY  = Profiler
TARGET    = $(SUBPLUGIN)-$(CATEGORY)

OBJS      = $(SUBPLUGIN)$(CATEGORY).o
DOCS      = README


ROOTBUILDDIR = ../../..
-include ../../../Make.config

all:
	@$(MAKE) -f ../../../Makefile.plugins -C "$(shell pwd)" _all

clean:
	@$(MAKE) -f ../../../Makefile.plugins -C "$(shell pwd)" _clean

install:
	@$(MAKE) -f ../../../Makefile.plugins -C "$(shell pwd)" _install
	@mkdir -p $(DESTDIR)$(VDRCFGDIR)
	cp --remove-destination channelTitle.conf $(DESTDIR)$(VDRCFGDIR)

uninstall:
	@$(MAKE) -f ../../../Makefile.plugins -C "$(shell pwd)" _uninstall
