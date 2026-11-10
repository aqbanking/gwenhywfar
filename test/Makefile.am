AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 $(LIBTLS_CFLAGS) $(LIBGCRYPT_CFLAGS) $(GPG_ERROR_CFLAGS) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@

noinst_HEADERS=\
  testthread.h \
  testthread_p.h


noinst_PROGRAMS=gwentest
gwentest_SOURCES=gwentest.c testthread.c
gwentest_LDADD=$(top_builddir)/src/libgwenhywfar.la \
  $(LIBGCRYPT_LIBS) $(LIBTLS_LIBS) $(GPG_ERROR_LIBS) \
  $(top_builddir)/src/base/libbase.la

TESTS = tls-tests.sh

CLEANFILES = tls-record.txt

EXTRA_DIST = README.MSVC gwentest.vcproj testdialog.xml \
  tls-tests.sh \
  tls-survey.sh \
  testdata/tls/mkcerts.sh \
  testdata/tls/ca.crt \
  testdata/tls/server.crt \
  testdata/tls/server.key \
  testdata/tls/expired.crt \
  testdata/tls/selfsigned.crt

# "make installcheck" runs gwentest from installcheck/, which is not in
# SUBDIRS, so that "make check" does not enter it.
SUBDIRS =
DIST_SUBDIRS = installcheck

installcheck-local:
	cd installcheck && $(MAKE) $(AM_MAKEFLAGS) installcheck

sources:
