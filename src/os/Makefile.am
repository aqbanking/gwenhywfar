INCLUDES=@all_includes@

SUBDIRS=@GWEN_OS@
DIST_SUBDIRS=posix windows

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 directory.h \
 inetaddr.h \
 inetsocket.h \
 libloader.h \
 process.h

noinst_headers= \
 inetsocket_all_p.h \
 inetsocket_l.h

noinst_LTLIBRARIES=libos.la
libos_la_SOURCES=\
 directory_all.c \
 inetsocket_all.c
 
libos_la_LIBADD=@GWEN_OS@/libosimpl.la

