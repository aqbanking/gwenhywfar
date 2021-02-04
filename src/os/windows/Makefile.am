AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/sio \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ @WERROR@ -DBUILDING_GWENHYWFAR

# We need to skip -I$(top_srcdir)/src/os here and instead specify the
# include path directly in the C files because when we add that
# directory to the include path, our file process.h shadows the
# mingw's version of that header file (process.h necessary for getpid
# on mingw).

noinst_LTLIBRARIES=libosimpl.la

noinst_HEADERS=\
 directory_p.h \
 fslock_p.h \
 gwentime_p.h \
 gwenthread_p.h \
 threadlocaldata_p.h \
 inetaddr_p.h \
 inetsocket_p.h \
 libloader_p.h \
 process_p.h \
 mutex_p.h \
 semaphore_p.h \
 syncio_file_l.h \
 syncio_file_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
  system.h

libosimpl_la_SOURCES=\
 directory.c \
 fslock.c \
 gwentime.c \
 gwenthread.c \
 threadlocaldata.c \
 inetaddr.c \
 inetsocket.c \
 init.c \
 libloader.c \
 process.c \
 mutex.c \
 semaphore.c \
 syncio_file.c


sources:
	for f in $(libosimpl_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

