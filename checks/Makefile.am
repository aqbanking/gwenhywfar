AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@

SUBDIRS = consumer

noinst_PROGRAMS=checks check_urlfns check_text
checks_SOURCES=\
 main.c \
 db.c \
 syncio_tls.c \
 syncio_http.c \
 syncio_https.c

check_urlfns_SOURCES = check_urlfns.c
check_text_SOURCES = check_text.c


noinst_HEADERS = checks.h

checks_LDADD=$(top_builddir)/src/libgwenhywfar.la 
check_urlfns_LDADD = $(top_builddir)/src/libgwenhywfar.la
check_text_LDADD = $(top_builddir)/src/libgwenhywfar.la

TESTS = checks check_urlfns check_text

sources:

