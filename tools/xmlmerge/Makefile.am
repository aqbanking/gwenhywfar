INCLUDES=@all_includes@ -I$(top_builddir)/gwenhywfar2

bin_PROGRAMS=xmlmerge
noinst_HEADERS=args.h
xmlmerge_SOURCES=args.c main.c
xmlmerge_LDADD=\
  $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(PROG_LIBRARIES)
