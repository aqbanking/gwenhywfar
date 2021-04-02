INCLUDES=@all_includes@ -I$(top_builddir) -I../ -I../../ -I$(gwenhywfar_symlinkdir)

DEFS=-DBUILDERDATADIR=\"$(datadir)/gwenhywfar/gwenbuild/builders\"


noinst_LTLIBRARIES=libgwb_builders.la

libgwb_builders_la_SOURCES=\
 genericbuilder.c


noinst_HEADERS=\
 genericbuilder.h \
 genericbuilder_p.h


builderdatadir=$(datadir)/gwenhywfar/gwenbuild/builders
dist_builderdata_DATA=\
  cbuilder.gwb \
  sharedlib.gwb \
  staticlib.gwb \
  tmplib.gwb \
  tm2builder.gwb


