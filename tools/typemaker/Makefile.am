INCLUDES=@all_includes@ -I..

localedir=$(datadir)/locale
DEFS=-D LOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS=typemaker
noinst_HEADERS=args.h
typemaker_SOURCES=args.c main.c phc.c lhc.c ahc.c code_c.c
typemaker_LDADD=-L/mingw/lib -L../../src @gwenhywfar_libs@

