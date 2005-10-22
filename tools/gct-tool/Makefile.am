INCLUDES=@all_includes@ -I$(top_builddir)

#EXTRA_DIST=

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS=gct-tool

noinst_HEADERS=\
  globals.h \
  cryptmanager_l.h \
  cryptmanager_p.h

gct_tool_SOURCES=\
  cryptmanager.c \
  signfiles.c \
  create.c \
  showctx.c \
  readkey.c \
  showuser.c \
  changepin.c \
  checktoken.c \
  disablekey.c \
  main.c

gct_tool_LDADD = \
  $(top_builddir)/src/libgwenhywfar.la \
  $(PROG_LIBRARIES)

