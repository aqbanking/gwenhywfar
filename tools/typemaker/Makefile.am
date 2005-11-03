INCLUDES=@all_includes@ -I$(top_builddir)/gwenhywfar2

EXTRA_DIST=test.xml

bin_PROGRAMS=typemaker
noinst_HEADERS=args.h typemaker_p.h
typemaker_SOURCES=args.c main.c phc.c lhc.c ahc.c code_c.c
typemaker_LDADD = \
  $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(PROG_LIBRARIES)
