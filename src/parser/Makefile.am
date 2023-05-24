AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ @WERROR@ -DBUILDING_GWENHYWFAR

typefiles=url.xml
EXTRA_DIST=$(typefiles) dbrw.c xmlrw.c xmlglobalize.c README.xml2db

noinst_LTLIBRARIES=libparser.la

noinst_HEADERS=\
 db_p.h \
 dbio_l.h \
 dbio_p.h \
 configmgr_l.h \
 configmgr_p.h \
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
 xsdnode_p.h \
 json_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 base64.h \
 configmgr.h \
 configmgr_be.h \
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
 xsdnode.h \
 xml2db.h \
 json.h \
 json_read.h \
 json_dump.h

libparser_la_SOURCES=\
 base64.c \
 configmgr.c \
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
 xsdnode.c \
 xml2db.c \
 json.c \
 json_read.c \
 json_dump.c



sources:
	for f in $(libparser_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


