INCLUDES= $(all_includes) -I$(top_builddir) -I$(top_srcdir)

#EXTRA_DIST=

bin_PROGRAMS=gct-tool

noinst_HEADERS=\
  globals.h \
  cryptmanager_l.h \
  cryptmanager_p.h

gct_tool_SOURCES=\
  cryptmanager.c \
  create.c \
  showctx.c \
  readkey.c \
  showuser.c \
  changepin.c \
  checktoken.c \
  disablekey.c \
  main.c

gct_tool_LDADD = \
  $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(PROG_LIBRARIES)

sources:
	for f in $(gct_tool_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

