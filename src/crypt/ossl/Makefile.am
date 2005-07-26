
#EXTRA_DIST = 

INCLUDES= $(all_includes) -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui

localedir=$(datadir)/locale
DEFS += \
  -DLOCALEDIR=\"$(localedir)\" \
  -DGWENHYWFAR_PLUGINS=\"$(gwenhywfar_plugindir)\" \
  -DPUBLIC_CA_FILE=\"$(sysconfdir)/gwen-public-ca.crt\"

certdir=$(sysconfdir)
dist_cert_DATA=gwen-public-ca.crt

noinst_LTLIBRARIES=libcryptimpl.la


noinst_HEADERS=\
 cryptssl_p.h \
 cryptssl_bf_p.h \
 cryptssl_des_p.h \
 cryptssl_md5_p.h \
 cryptssl_sha1_p.h \
 cryptssl_rmd160_p.h \
 cryptssl_rsa_p.h \
 nettransportssl_p.h

libcryptimpl_la_SOURCES=\
 cryptssl_bf.c \
 cryptssl_des.c \
 cryptssl_md5.c \
 cryptssl_sha1.c \
 cryptssl_rmd160.c \
 cryptssl_rsa.c \
 cryptssl.c \
 nettransportssl.c

