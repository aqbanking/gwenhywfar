INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

typefiles=ssl_cert_descr.xml
EXTRA_DIST=$(typefiles)

noinst_LTLIBRARIES=libsio.la

noinst_HEADERS=\
 syncio_p.h \
 syncio_socket_p.h \
 syncio_memory_p.h \
 syncio_buffered_p.h \
 syncio_tls_p.h \
 syncio_http_p.h \
 ssl_cert_descr_l.h \
 ssl_cert_descr_p.h \
 httpsession_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 syncio.h \
 syncio_file.h \
 syncio_socket.h \
 syncio_memory.h \
 syncio_buffered.h \
 syncio_tls.h \
 syncio_http.h \
 ssl_cert_descr.h \
 ssl_cert_flags.h \
 httpsession.h

libsio_la_SOURCES=\
 syncio.c \
 syncio_socket.c \
 syncio_memory.c \
 syncio_buffered.c \
 syncio_tls.c \
 syncio_http.c \
 ssl_cert_descr.c \
 httpsession.c


types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)


sources:
	for f in $(libsio_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

