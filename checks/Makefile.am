INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@

noinst_PROGRAMS=checks check_urlfns
checks_SOURCES=\
 main.c \
 db.c \
 io_file.c \
 io_buffered.c \
 io_socket.c \
 io_tls.c \
 io_http.c \
 io_https.c \
 io_packets.c \
 syncio_tls.c \
 syncio_http.c

check_urlfns_SOURCES = check_urlfns.c


noinst_HEADERS = checks.h

checks_LDADD=$(top_builddir)/src/libgwenhywfar.la 
check_urlfns_LDADD = $(top_builddir)/src/libgwenhywfar.la

TESTS = checks check_urlfns

sources:

