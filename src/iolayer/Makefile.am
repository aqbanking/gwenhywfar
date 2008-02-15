INCLUDES= -I$(gwenhywfar_symlinkdir) @LIBGNUTLS_CFLAGS@ \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt3 \
 -I$(top_srcdir)/src/parser

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

typefiles=ssl_cert_descr.xml
EXTRA_DIST=$(typefiles)

noinst_LTLIBRARIES=libiolayer.la

noinst_HEADERS=\
 iolayer_l.h \
 iolayer_p.h \
 iomanager_l.h \
 iorequest_l.h \
 iorequest_p.h \
 io_buffered_p.h \
 io_codec_p.h \
 io_file_p.h \
 io_http_p.h \
 io_memory_p.h \
 io_packets_p.h \
 io_socket_p.h \
 io_tls_p.h \
 ssl_cert_descr_l.h \
 ssl_cert_descr_p.h \
 httpsession_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 iolayer.h \
 iolayer_be.h \
 iomanager.h \
 iorequest.h \
 io_buffered.h \
 iorequest_be.h \
 io_codec_be.h \
 io_file.h \
 io_http.h \
 io_memory.h \
 io_packets.h \
 io_socket.h \
 io_tls.h \
 ssl_cert_descr.h \
 ssl_cert_flags.h \
 httpsession.h

libiolayer_la_SOURCES=\
 iolayer.c \
 iomanager.c \
 iorequest.c \
 io_buffered.c \
 io_codec.c \
 io_file.c \
 io_http.c \
 io_memory.c \
 io_packets.c \
 io_socket.c \
 io_tls.c \
 ssl_cert_descr.c \
 httpsession.c


types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)


sources:
	for f in $(libio_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

