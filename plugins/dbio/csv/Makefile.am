EXTRA_DIST=README

dbioplugindir=@gwenhywfar_plugins@/dbio

dbioplugin_LTLIBRARIES=csv.la
noinst_HEADERS=csv_p.h

csv_la_SOURCES=csv.c
csv_la_LIBADD=-L../../../src -lgwenhywfar
csv_la_LDFLAGS=@STRIPALL@ -module -version-info 0:1:0

