INCLUDES=@all_includes@

AM_CFLAGS=-DGWENHYWFAR_PLUGINS=\"@gwenhywfar_plugins@\"

noinst_LTLIBRARIES=libparser.la

noinst_HEADERS=\
 csv_p.h \
 db_p.h \
 dbio_l.h \
 dbio_p.h \
 msgengine_p.h \
 xml_p.h

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 csv.h \
 db.h \
 dbio.h \
 msgengine.h \
 path.h \
 text.h \
 xml.h

libparser_la_SOURCES=\
 csv.c \
 db.c \
 dbio.c \
 msgengine.c \
 path.c \
 text.c \
 xml.c

