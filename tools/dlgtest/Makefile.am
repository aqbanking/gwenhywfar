INCLUDES=@all_includes@ -I$(top_builddir)

bin_PROGRAMS=dlgtest
#noinst_HEADERS=args.h
dlgtest_SOURCES=main.c
dlgtest_LDADD = \
  $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(top_builddir)/gwenui/libgwenui.la \
  $(PROG_LIBRARIES)
dlgtest_LDFLAGS=@i18n_libs@

