#
# Makefile for a UPnP subplugin
#
# $Id$
#

SUBPLUGIN = rec
CATEGORY  = Provider
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

uninstall:
	@$(MAKE) -f ../../../Makefile.plugins -C "$(shell pwd)" _uninstall
