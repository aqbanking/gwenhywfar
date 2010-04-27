INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

noinst_LTLIBRARIES=libsio.la

noinst_HEADERS=\
 syncio_p.h \
 syncio_socket_p.h \
 syncio_memory_p.h \
 syncio_buffered_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 syncio.h \
 syncio_file.h \
 syncio_socket.h \
 syncio_memory.h \
 syncio_buffered.h

libsio_la_SOURCES=\
 syncio.c \
 syncio_socket.c \
 syncio_memory.c \
 syncio_buffered.c


sources:
	for f in $(libsio_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

