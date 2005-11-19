INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

typefiles=url.xml
EXTRA_DIST=$(typefiles)

noinst_LTLIBRARIES=libnet2.la

noinst_HEADERS=\
  net2_l.h \
  netlayer_l.h \
  netlayer_p.h \
  nl_file_p.h \
  nl_hbci_p.h \
  nl_http_p.h \
  nl_log_p.h \
  nl_packets_p.h \
  nl_socket_p.h \
  url_l.h \
  url_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
  netlayer.h \
  nl_socket.h \
  nl_http.h \
  url.h urlfns.h \
  net2.h \
  nl_hbci.h \
  nl_packets.h \
  nl_file.h \
  nl_log.h

libnet2_la_SOURCES=\
  netlayer.c \
  nl_socket.c \
  nl_http.c \
  url.c urlfns.c \
  net2.c \
  nl_hbci.c \
  nl_packets.c \
  nl_file.c \
  nl_log.c

types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

sources:
	for f in $(libnet2_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


