INCLUDES=-I$(gwenhywfar_symlinkdir)

ABSMEDIAPATH=$(abs_srcdir)

DEFS+=-DMEDIAPATH=\"$(ABSMEDIAPATH)\"

noinst_LTLIBRARIES=libgwengui-test.la

libgwengui_test_la_SOURCES = \
  dlg_test.c \
  dlg_test2.c

libgwengui_test_la_LIBADD = $(top_builddir)/src/$(gwenhywfar_internal_libname)

noinst_HEADERS=\
  dlg_test.h \
  dlg_test2.h


EXTRA_DIST=\
  dlg_test.dlg \
  disk.png

