INCLUDES= -I$(top_builddir) -I$(top_srcdir) -I$(gwenhywfar_symlinkdir) $(ssl_includes)

#EXTRA_DIST=

bin_PROGRAMS=gct-tool

noinst_HEADERS=\
  globals.h

gct_tool_SOURCES=\
  create.c \
  genkey.c \
  showkey.c \
  showuser.c \
  update.c \
  setsignseq.c \
  main.c

if IS_WINDOWS

gct_tool_LDFLAGS=-L$(top_builddir)/src/.libs -mconsole
#gct_tool_LDADD= -lgwenhywfar-$(GWENHYWFAR_SO_EFFECTIVE) $(PROG_LIBRARIES)
gct_tool_LDADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)

else

gct_tool_LDADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)

endif


if IF_HAVE_OPENSSL
gct_tool_LDADD+=$(ssl_libraries) $(ssl_lib)
endif

sources:
	for f in $(gct_tool_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done
