INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui
 
localedir=$(datadir)/locale
DEFS=-DLOCALEDIR=\"$(localedir)\" @DEFS@

noinst_LTLIBRARIES=libipc.la

noinst_HEADERS=\
 ipc_p.h \
 netconnectionhttp_p.h

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 ipc.h \
 netconnectionhttp.h

libipc_la_SOURCES=\
 ipc.c \
 netconnectionhttp.c

