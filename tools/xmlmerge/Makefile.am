INCLUDES=@all_includes@

localedir=$(datadir)/locale
DEFS=-DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS=xmlmerge
noinst_HEADERS=args.h
xmlmerge_SOURCES=args.c main.c
xmlmerge_LDADD= @PROG_LIBRARIES@ \
  -L/mingw/lib -L../../src -lgwenhywfar 
