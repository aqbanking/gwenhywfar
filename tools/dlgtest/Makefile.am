INCLUDES=@all_includes@

localedir=$(datadir)/locale
DEFS=-D LOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS=dlgtest
#noinst_HEADERS=args.h
dlgtest_SOURCES=main.c
dlgtest_LDADD=\
  -L/mingw/lib -L../../src @gwenhywfar_libs@ \
  -L../../gwenui @gwenhywfar_libsui@
