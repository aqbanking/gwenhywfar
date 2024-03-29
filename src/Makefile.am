AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/html \
 -I$(top_srcdir)/src/parser \
 $(LIBGNUTLS_CFLAGS) \
 $(all_includes)

AM_CFLAGS=$(visibility_cflags) @WERROR@

SUBDIRS = base sio os parser crypt3 crypttoken gui cryptmsg html sar xmlcmd msgio test_framework
DIST_SUBDIRS=base sio os parser crypt3 crypttoken gui cryptmsg html sar xmlcmd msgio test_framework

DEFS += \
  -DBUILDING_GWENHYWFAR \
  -DGWEN_PREFIX_DIR=\"$(prefix)\" \
  -DGWEN_SYSCONF_DIR=\"@gwenhywfar_cfg_searchdir@\" \
  -DLOCALEDIR=\"@gwenhywfar_locale_searchdir@\" \
  -DLIBDIR=\"$(libdir)\" \
  -DGWENHYWFAR_SO_EFFECTIVE_STR=\"$(GWENHYWFAR_SO_EFFECTIVE)\" \
  -DGWEN_DATADIR=\"$(gwenhywfar_data_searchdir)\" \
  -DGWEN_SYSDATADIR=\"$(gwenhywfar_sysdata_searchdir)\" \
  -DPLUGINDIR=\"$(gwenhywfar_plugin_searchdir)\"

# Note: The symbol "DATADIR" is already used by some Windows header,
# so we have to name this differently here.

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

noinst_HEADERS=\
  binreloc.h

lib_LTLIBRARIES=libgwenhywfar.la

libgwenhywfar_la_SOURCES=\
  binreloc.c \
  gwenhywfar.c

libgwenhywfar_la_LIBADD=\
  base/libbase.la \
  crypt3/libcrypt3.la \
  crypttoken/libcrypttoken.la \
  cryptmsg/libcryptmsg.la \
  sio/libsio.la \
  os/libos.la \
  parser/libparser.la \
  html/libgwenhtml.la \
  gui/libgui.la \
  sar/libsar.la \
  xmlcmd/libxmlcmd.la \
  msgio/libgwenmsgio.la \
  test_framework/libtestframework.la $(i18n_libs) $(PTHREAD_CFLAGS) $(PTHREAD_LIBS) $(ICONV_LIBS) $(LIBGCRYPT_LIBS) $(LIBGNUTLS_LIBS) $(LIBXML_LIBS) $(all_libraries)

libgwenhywfar_la_CFLAGS=$(PTHREAD_CFLAGS)

noinst_PROGRAMS=testlib
testlib_SOURCES=testlib.c
testlib_LDADD=libgwenhywfar.la

libgwenhywfar_la_LDFLAGS= \
  -no-undefined $(STRIPALL) $(ressource_ldflags) \
  -version-info \
   $(GWENHYWFAR_SO_CURRENT):$(GWENHYWFAR_SO_REVISION):$(GWENHYWFAR_SO_AGE)


TESTS = testlib

sources:
	for f in $(libgwenhywfar_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done ;


ressource.o: ressource.rc
	$(RC) -o $@ $<


