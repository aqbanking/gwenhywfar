#EXTRA_DIST=README

dbioplugindir=@gwenhywfar_plugins@/dbio

localedir=$(datadir)/locale
DEFS=-DLOCALEDIR=\"$(localedir)\" @DEFS@

dbioplugin_LTLIBRARIES=olddb.la
dbioplugin_DATA=olddb.xml
noinst_HEADERS=olddb_p.h

olddb_la_SOURCES=olddb.c
olddb_la_LIBADD=-L$(top_builddir)/src -lgwenhywfar
olddb_la_LDFLAGS=@STRIPALL@ -module -version-info 0:1:0

