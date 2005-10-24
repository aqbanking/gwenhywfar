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

if IS_WINDOWS
  PROVIDED_GWEN_LIB=$(top_builddir)/src/gwenhywfar32_$(GWENHYWFAR_SO_EFFECTIVE).dll
else
  PROVIDED_GWEN_LIB=$(top_builddir)/src/libgwenhywfar.la
endif

gct_tool_LDADD = \
  $(PROVIDED_GWEN_LIB) \
  $(PROG_LIBRARIES)

