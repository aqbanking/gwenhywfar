INCLUDES=@all_includes@ -I$(top_builddir)

bin_PROGRAMS=xmlmerge
noinst_HEADERS=args.h
xmlmerge_SOURCES=args.c main.c
xmlmerge_LDADD=\
  $(top_builddir)/src/libgwenhywfar.la \
  $(PROG_LIBRARIES)
