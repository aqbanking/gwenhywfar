INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui
 

noinst_LTLIBRARIES=libnet.la

noinst_HEADERS=\
 netconnection_p.h \
 net_l.h \
 netmsg_p.h \
 nettransport_p.h \
 nettransportsock_p.h

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 net.h \
 netconnection.h \
 netmsg.h \
 nettransport.h \
 nettransportsock.h

libnet_la_SOURCES=\
 net.c \
 netconnection.c \
 netmsg.c \
 nettransport.c \
 nettransportsock.c

