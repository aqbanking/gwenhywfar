AM_CPPFLAGS= -I$(top_builddir) -I$(top_srcdir) -I$(gwenhywfar_symlinkdir)
if IF_HAVE_OPENSSL
AM_CPPFLAGS+= $(OPENSSL_CFLAGS)
endif

#EXTRA_DIST=

bin_PROGRAMS=gct-tool

noinst_HEADERS=\
  globals.h

gct_tool_SOURCES=\
  activatekey.c \
  changepin.c \
  create.c \
  genkey.c \
  showkey.c \
  showuser.c \
  update.c \
  setsignseq.c \
  setkey.c \
  hashtree.c \
  checktree.c \
  showpasswords.c \
  main.c

gct_tool_LDADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)

if IS_WINDOWS

gct_tool_LDFLAGS=-mconsole
gct_tool_LDADD+=$(PROG_LIBRARIES)

endif

noinst_PROGRAMS=cttest
cttest_SOURCES=cttest.c
cttest_LDADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)


if IF_HAVE_OPENSSL
gct_tool_LDADD+=$(OPENSSL_LIBS)
endif

sources:
	for f in $(gct_tool_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

