INCLUDES=@all_includes@ -I$(top_builddir)

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS=dlgtest
#noinst_HEADERS=args.h
dlgtest_SOURCES=main.c
dlgtest_LDADD = \
  -L$(top_builddir)/src $(gwenhywfar_libs) \
  -L$(top_builddir)/gwenui $(gwenhywfar_libsui) \
  -L/mingw/lib $(PROG_LIBRARIES)
