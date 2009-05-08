INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

noinst_LTLIBRARIES=libio.la

noinst_HEADERS=\
 bio_buffer_p.h \
 bio_file_p.h \
 bio_socket_p.h \
 bufferedio_l.h \
 bufferedio_p.h \
 filter_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 bio_buffer.h \
 bio_file.h \
 bio_socket.h \
 bufferedio.h \
 filter.h

libio_la_SOURCES=\
 bio_buffer.c \
 bio_file.c \
 bio_socket.c \
 bufferedio.c \
 filter.c


sources:
	for f in $(libio_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

