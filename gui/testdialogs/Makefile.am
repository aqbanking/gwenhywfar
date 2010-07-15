INCLUDES=-I$(gwenhywfar_symlinkdir)

ABSMEDIAPATH="$(shell pwd)"

DEFS+=-DMEDIAPATH=\"$(ABSMEDIAPATH)\"

noinst_LTLIBRARIES=libgwengui-test.la

libgwengui_test_la_SOURCES = dlg_test.c
libgwengui_test_la_LIBADD = $(top_builddir)/src/$(gwenhywfar_internal_libname)

noinst_HEADERS=\
  dlg_test.h


EXTRA_DIST=\
  dlg_test.dlg \
  disk.png

