INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

EXTRA_DIST=README

noinst_LTLIBRARIES=libosimpl.la

noinst_HEADERS=\
 directory_p.h \
 fslock_p.h \
 gwentime_p.h \
 inetaddr_p.h \
 inetsocket_p.h \
 libloader_p.h \
 process_p.h \
 mutex_p.h \
 semaphore_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
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
 process.c \
 mutex.c \
 semaphore.c


sources:
	for f in $(libosimpl_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


