INCLUDES=-I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui @all_includes@


localedir=$(datadir)/locale
DEFS=-DLOCALEDIR=\"$(localedir)\" @DEFS@

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
  crypttoken.h

noinst_LTLIBRARIES=libcrypt2.la
noinst_HEADERS=\
 crypttoken_p.h

libcrypt2_la_SOURCES=\
 crypttoken.c
