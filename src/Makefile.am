## Process this file with automake to produce Makefile.in

INCLUDES = $(PX_INCLUDEDIR) $(GSF_INCLUDEDIR) -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\"

bin_PROGRAMS = pxindex

pxindex_SOURCES = main.c

pxindex_LDADD = $(PX_LIBDIR) $(PX_LIBS) $(GSF_LIBDIR) $(GSF_LIBS)
