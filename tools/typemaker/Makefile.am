INCLUDES=@all_includes@ -I$(top_builddir)

EXTRA_DIST=test.xml

bin_PROGRAMS=typemaker
noinst_HEADERS=args.h typemaker_p.h
typemaker_SOURCES=args.c main.c phc.c lhc.c ahc.c code_c.c
typemaker_LDADD = \
  -L$(top_builddir)/src $(gwenhywfar_libs) \
  -L/mingw/lib $(PROG_LIBRARIES)
