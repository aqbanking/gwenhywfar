INCLUDES=@all_includes@ -I..

EXTRA_DIST=test.xml

localedir=$(datadir)/locale
DEFS=-DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS=typemaker
noinst_HEADERS=args.h typemaker_p.h
typemaker_SOURCES=args.c main.c phc.c lhc.c ahc.c code_c.c
typemaker_LDADD=@PROG_LIBRARIES@ \
  -L/mingw/lib -L../../src -lgwenhywfar
