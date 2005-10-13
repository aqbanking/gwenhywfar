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

typefiles=request.xml
EXTRA_DIST=$(typefiles)


noinst_LTLIBRARIES=libipcrequest.la

noinst_HEADERS=\
  request_l.h \
  request_p.h \
  requestmgr_p.h

iheaderdir=$(pkgincludedir)
iheader_HEADERS=\
  request.h \
  requestmgr.h

libipcrequest_la_SOURCES=\
  request.c \
  requestmgr.c


types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

