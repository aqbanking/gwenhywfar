INCLUDES=@all_includes@ -I$(top_builddir)

bin_PROGRAMS=dlgtest
#noinst_HEADERS=args.h
dlgtest_SOURCES=main.c
dlgtest_LDADD = \
  $(top_builddir)/src/libgwenhywfar.la \
  $(top_builddir)/gwenui/libgwenui.la \
  $(PROG_LIBRARIES)
