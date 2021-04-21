SUBDIRS=posix windows

INCLUDES=@all_includes@ -I$(top_builddir) -I../ -I../../ -I$(gwenhywfar_symlinkdir)

DEFS=-DBUILDERDATADIR=\"$(datadir)/gwenhywfar/gwenbuild/builders\"


noinst_LTLIBRARIES=libgwb_builders.la

libgwb_builders_la_SOURCES=\
 genericbuilder.c \
 gbuilderdescr.c


noinst_HEADERS=\
 genericbuilder.h \
 genericbuilder_p.h \
 gbuilderdescr.h \
 gbuilderdescr_p.h


builderdatadir=$(datadir)/gwenhywfar/gwenbuild/builders
dist_builderdata_DATA=\
  app.gwb \
  cxxapp.gwb \
  cbuilder.gwb \
  cxxbuilder.gwb \
  module.gwb \
  sharedlib.gwb \
  staticlib.gwb \
  tmplib.gwb \
  tm2builder.gwb \
  windowssharedlib.gwb \
  windowsapp.gwb


