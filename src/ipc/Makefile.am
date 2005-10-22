SUBDIRS=request

INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui

AM_CFLAGS=@PROFILING_CFLAGS@

typefiles=httpcookie.xml httpurl.xml
EXTRA_DIST=$(typefiles)

noinst_LTLIBRARIES=libipc.la

noinst_HEADERS=\
 ipc_p.h \
 netconnectionhttp_p.h \
 httpsession_p.h \
 httpcookie_l.h \
 httpcookie_p.h \
 httpurl_l.h \
 httpurl_p.h

iheaderdir=$(pkgincludedir)
iheader_HEADERS=\
 ipc.h \
 netconnectionhttp.h \
 httpsession.h \
 httpcookie.h \
 httpurl.h \
 httpurlfns.h

libipc_la_SOURCES=\
 ipc.c \
 netconnectionhttp.c \
 httpsession.c \
 httpcookie.c \
 httpurl.c \
 httpurlfns.c

libipc_la_LIBADD=request/libipcrequest.la

types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

