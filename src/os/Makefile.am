INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui
 

SUBDIRS=@GWEN_OS@
DIST_SUBDIRS=posix windows

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 directory.h \
 gwentime.h \
 inetaddr.h \
 inetsocket.h \
 libloader.h \
 process.h

noinst_HEADERS= \
 inetaddr_l.h \
 inetsocket_all_p.h \
 inetsocket_l.h \
 libloader_l.h \
 process_l.h

noinst_LTLIBRARIES=libos.la
libos_la_SOURCES=\
 directory_all.c \
 inetsocket_all.c
 
libos_la_LIBADD=@GWEN_OS@/libosimpl.la

