## Process this file with automake to produce Makefile.in

SUBDIRS = po src $(DOCDIR)

spec = $(PACKAGE).spec

EXTRA_DIST = config.rpath mkinstalldirs  config.rpath mkinstalldirs  config.rpath m4/ChangeLog  COPYING INSTALL intltool-extract.in intltool-merge.in intltool-update.in $(spec) $(spec).in autogen.sh

rpm: $(distdir).tar.gz
	rpm -ta $(distdir).tar.gz


ACLOCAL_AMFLAGS = -I m4
