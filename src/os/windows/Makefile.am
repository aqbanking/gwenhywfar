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

noinst_LTLIBRARIES=libosimpl.la

noinst_HEADERS=\
 directory_p.h \
 fslock_p.h \
 gwentime_p.h \
 inetaddr_p.h \
 inetsocket_p.h \
 libloader_p.h \
 process_p.h

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
  system.h

libosimpl_la_SOURCES=\
 directory.c \
 fslock.c \
 gwentime.c \
 inetaddr.c \
 inetsocket.c \
 init.c \
 libloader.c \
 process.c
