INCLUDES=@all_includes@ -I$(top_builddir)

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS=dlgtest
#noinst_HEADERS=args.h
dlgtest_SOURCES=main.c
dlgtest_LDADD = \
  -L/mingw/lib -L$(top_builddir)/src $(gwenhywfar_libs) $(PROG_LIBRARIES) \
  -L$(top_builddir)/gwenui $(gwenhywfar_libsui)
