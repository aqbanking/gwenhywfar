SUBDIRS=request

INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

noinst_LTLIBRARIES=libipc.la

noinst_HEADERS=\
 ipc_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 ipc.h

libipc_la_SOURCES=\
 ipc.c

libipc_la_LIBADD=request/libipcrequest.la

