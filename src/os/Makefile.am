AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ @WERROR@ -DBUILDING_GWENHYWFAR

SUBDIRS=@GWEN_OS@
DIST_SUBDIRS=posix windows

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 directory.h \
 fslock.h \
 gwentime.h \
 gwenthread.h \
 threadlocaldata.h \
 inetaddr.h \
 inetsocket.h \
 libloader.h \
 process.h \
 mutex.h \
 gwensemaphore.h \
 gwendate.h

noinst_HEADERS= \
 gwendate_p.h \
 gwentime_l.h \
 gwentime_p.h \
 inetaddr_l.h \
 inetsocket_l.h \
 libloader_l.h \
 process_l.h

noinst_LTLIBRARIES=libos.la
libos_la_SOURCES=\
 directory_all.c \
 gwentime_all.c \
 gwendate.c \
 process_all.c


libos_la_LIBADD=@GWEN_OS@/libosimpl.la

sources:
	for f in $(libos_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

