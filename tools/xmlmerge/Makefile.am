INCLUDES=@all_includes@ -I$(top_builddir)

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS=xmlmerge
noinst_HEADERS=args.h
xmlmerge_SOURCES=args.c main.c
xmlmerge_LDADD= $(PROG_LIBRARIES) \
  -L/mingw/lib -L$(top_builddir)/src $(gwenhywfar_libs)
