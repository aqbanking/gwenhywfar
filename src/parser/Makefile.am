INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui
 

AM_CFLAGS=-DGWENHYWFAR_PLUGINS=\"@gwenhywfar_plugins@\"

noinst_LTLIBRARIES=libparser.la

noinst_HEADERS=\
 csv_p.h \
 db_p.h \
 dbio_l.h \
 dbio_p.h \
 msgengine_p.h \
 tlv_p.h \
 xml_p.h

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 base64.h \
 csv.h \
 db.h \
 dbio.h \
 msgengine.h \
 path.h \
 text.h \
 tlv.h \
 xml.h

libparser_la_SOURCES=\
 base64.c \
 csv.c \
 db.c \
 dbio.c \
 msgengine.c \
 path.c \
 text.c \
 tlv.c \
 xml.c

