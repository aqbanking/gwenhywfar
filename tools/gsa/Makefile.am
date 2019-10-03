AM_CPPFLAGS= -I$(top_builddir) -I$(top_srcdir) -I$(gwenhywfar_symlinkdir) $(ssl_includes)

#EXTRA_DIST=

bin_PROGRAMS=gsa

noinst_HEADERS=\
  globals.h

gsa_SOURCES=\
  add.c \
  create.c \
  check.c \
  extract.c \
  list.c \
  mkkey.c \
  rfi.c \
  sign.c \
  verify.c \
  main.c

gsa_LDADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)

if IS_WINDOWS

gsa_LDFLAGS=-mconsole
gsa_LDADD+=$(PROG_LIBRARIES)

endif


sources:
	for f in $(gsa_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

