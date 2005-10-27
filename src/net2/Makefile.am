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

typefiles=url.xml
EXTRA_DIST=$(typefiles)

noinst_LTLIBRARIES=libnet2.la

noinst_HEADERS=\
  netlayer_l.h \
  netlayer_p.h \
  nl_socket_p.h \
  nl_http_p.h \
  url_p.h \
  net2_l.h

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
  netlayer.h \
  nl_socket.h \
  nl_http.h \
  url.h urlfns.h \
  net2.h

libnet2_la_SOURCES=\
  netlayer.c \
  nl_socket.c \
  nl_http.c \
  url.c urlfns.c \
  net2.c

types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

