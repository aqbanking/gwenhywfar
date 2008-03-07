INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

typefiles=url.xml
EXTRA_DIST=$(typefiles) dbrw.c xmlrw.c xmlglobalize.c

noinst_LTLIBRARIES=libparser.la

noinst_HEADERS=\
 db_p.h \
 dbio_l.h \
 dbio_p.h \
 msgengine_p.h \
 tag16_p.h \
 tlv_p.h \
 url_l.h \
 url_p.h \
 xml_p.h \
 xml_l.h \
 xmlctx_l.h \
 xmlctx_p.h \
 xsdnode_l.h \
 xsdnode_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 base64.h \
 db.h \
 dbio.h \
 dbio_be.h \
 msgengine.h \
 path.h \
 tag16.h \
 text.h \
 tlv.h \
 url.h urlfns.h \
 xml.h \
 xmlctx.h \
 xsdnode.h

libparser_la_SOURCES=\
 base64.c \
 db.c \
 dbio.c \
 msgengine.c \
 path.c \
 tag16.c \
 text.c \
 tlv.c \
 url.c urlfns.c \
 xml.c \
 xmlctx.c \
 xsdnode.c



sources:
	for f in $(libparser_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


