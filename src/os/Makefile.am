INCLUDES=@all_includes@

EXTRA_DIST=\
 windows/Makefile.am \
 windows/Makefile.in \
 windows/directory.c \
 windows/directory_p.h \
 windows/inetaddr.c \
 windows/inetaddr_p.h \
 windows/inetsocket.c \
 windows/inetsocket_p.h \
 windows/libloader.c \
 windows/libloader_p.h \
 posix/Makefile.am \
 posix/Makefile.in \
 posix/directory_p.h \
 posix/inetaddr_p.h \
 posix/inetsocket_p.h \
 posix/libloader_p.h \
 posix/directory.c \
 posix/inetaddr.c \
 posix/inetsocket.c \
 posix/libloader.c

SUBDIRS=@GWEN_OS@

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 directory.h \
 inetaddr.h \
 inetsocket.h \
 libloader.h

noinst_LTLIBRARIES=libos.la
libos_la_SOURCES=\
 directory_all.c
 
libos_la_LIBADD=@GWEN_OS@/libosimpl.la

