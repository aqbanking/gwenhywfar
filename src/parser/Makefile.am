INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

EXTRA_DIST = xsdtypes.inc xsdtypes.xsd mkxsdtypes.sh

noinst_LTLIBRARIES=libparser.la

noinst_HEADERS=\
 csv_p.h \
 db_p.h \
 dbio_l.h \
 dbio_p.h \
 msgengine_p.h \
 tlv_p.h \
 xml_p.h \
 xml_l.h \
 xsd_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 base64.h \
 csv.h \
 db.h \
 dbio.h \
 msgengine.h \
 path.h \
 text.h \
 tlv.h \
 xml.h \
 xsd.h

libparser_la_SOURCES=\
 base64.c \
 csv.c \
 db.c \
 dbio.c \
 msgengine.c \
 path.c \
 text.c \
 tlv.c \
 xml.c \
 xsd.c xsd_list.c xsd_write.c



xsdtypes.inc: xsdtypes.xsd
	./mkxsdtypes.sh $< > $@

sources:
	for f in $(libparser_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


