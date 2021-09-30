SUBDIRS=posix windows

INCLUDES=@all_includes@ -I$(top_builddir) -I../ -I../../ -I$(top_srcdir)/tools -I$(gwenhywfar_symlinkdir)

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

