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
 windows/init.c \
 windows/libloader.c \
 windows/libloader_p.h \
 windows/process.c \
 windows/process_p.h \
 windows/system.h.in \
 posix/Makefile.am \
 posix/Makefile.in \
 posix/directory_p.h \
 posix/inetaddr_p.h \
 posix/inetsocket_p.h \
 posix/libloader_p.h \
 posix/directory.c \
 posix/inetaddr.c \
 posix/inetsocket.c \
 posix/libloader.c \
 posix/process.c \
 posix/process_p.h \
 posix/system.h.in

SUBDIRS=@GWEN_OS@

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 directory.h \
 inetaddr.h \
 inetsocket.h \
 libloader.h \
 process.h

noinst_LTLIBRARIES=libos.la
libos_la_SOURCES=\
 directory_all.c
 
libos_la_LIBADD=@GWEN_OS@/libosimpl.la

