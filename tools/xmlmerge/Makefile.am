INCLUDES=@all_includes@ -I$(top_builddir)

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS=xmlmerge
noinst_HEADERS=args.h
xmlmerge_SOURCES=args.c main.c
xmlmerge_LDADD=\
  -L$(top_builddir)/src $(gwenhywfar_libs) \
  -L/mingw/lib $(PROG_LIBRARIES)
