INCLUDES=@all_includes@

localedir=$(datadir)/locale
DEFS=-D LOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS=xmlmerge
noinst_HEADERS=args.h
xmlmerge_SOURCES=args.c main.c
xmlmerge_LDADD=-L/mingw/lib -L../../src @gwenhywfar_libs@
