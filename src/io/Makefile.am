INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui
 
localedir=$(datadir)/locale
DEFS=-D LOCALEDIR=\"$(localedir)\" @DEFS@

noinst_LTLIBRARIES=libio.la

noinst_HEADERS=\
 bio_buffer_p.h \
 bio_file_p.h \
 bio_socket_p.h \
 bufferedio_l.h \
 bufferedio_p.h

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 bio_buffer.h \
 bio_file.h \
 bio_socket.h \
 bufferedio.h

libio_la_SOURCES=\
 bio_buffer.c \
 bio_file.c \
 bio_socket.c \
 bufferedio.c
