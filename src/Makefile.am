INCLUDES= \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser $(all_includes)

AM_CFLAGS=$(visibility_cflags)

SUBDIRS = base crypt io ipc os parser crypt2 net2

localedir=$(datadir)/locale
DEFS += \
  -DBUILDING_GWENHYWFAR \
  -DGWEN_PREFIX_DIR=\"$(prefix)\" \
  -DGWEN_SYSCONF_DIR=\"$(sysconfdir)\" \
  -DLOCALEDIR=\"$(localedir)\" \
  -DLIBDIR=\"$(libdir)\" \
  -DPLUGINDIR=\"$(gwenhywfar_plugindir)\"

EXTRA_DIST=\
 types.h.in \
 version.h.in \
 ressource.rc.in ressource.rc

# Before compiling the sources, first built these files
BUILT_SOURCES = types.h version.h


iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
  gwenhywfar.h \
  gwenhywfarapi.h \
  types.h \
  version.h

lib_LTLIBRARIES=libgwenhywfar.la

libgwenhywfar_la_SOURCES=\
  gwenhywfar.c

libgwenhywfar_la_LIBADD=\
  base/libbase.la \
  crypt/libcrypt.la \
  crypt2/libcrypt2.la \
  io/libio.la \
  ipc/libipc.la \
  net2/libnet2.la \
  os/libos.la \
  parser/libparser.la $(ssl_libraries) $(ssl_lib) $(all_libraries)
libgwenhywfar_la_DEPENDENCIES=\
  base/libbase.la \
  crypt/libcrypt.la \
  crypt2/libcrypt2.la \
  io/libio.la \
  ipc/libipc.la \
  net2/libnet2.la \
  os/libos.la \
  parser/libparser.la

noinst_PROGRAMS=testlib
testlib_SOURCES=testlib.c
testlib_LDADD=libgwenhywfar.la

# if IS_WINDOWS
if IS_WINDOWS
libgwenhywfar_la_DEPENDENCIES += ressource.o
ressource_ldflags=-Wl,ressource.o
endif
# IS_WINDOWS

libgwenhywfar_la_LDFLAGS= \
  -no-undefined $(STRIPALL) $(ressource_ldflags) \
  -version-info \
   $(GWENHYWFAR_SO_CURRENT):$(GWENHYWFAR_SO_REVISION):$(GWENHYWFAR_SO_AGE)


TESTS = testlib

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

ressource.o: ressource.rc
	$(RC) -o $@ $<


