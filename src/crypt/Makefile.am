INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@PROFILING_CFLAGS@

SUBDIRS=@GWEN_CRYPTO@
DIST_SUBDIRS=none ossl

typefiles=ssl_cipher.xml ssl_cert_descr.xml
EXTRA_DIST=$(typefiles)

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 crypt.h \
 keymanager.h \
 keyspec.h \
 md.h \
 padd.h \
 nl_ssl.h \
 ssl_cipher.h \
 ssl_cert_descr.h

noinst_HEADERS=\
 crypt_l.h \
 crypt_p.h \
 keymanager_p.h \
 keyspec_p.h \
 md_p.h \
 padd_p.h \
 ssl_cipher_l.h \
 ssl_cipher_p.h \
 ssl_cert_descr_l.h \
 ssl_cert_descr_p.h

noinst_LTLIBRARIES=libcrypt.la

libcrypt_la_SOURCES=\
 crypt.c \
 keymanager.c \
 keyspec.c \
 md.c \
 padd.c \
 ssl_cipher.c \
 ssl_cert_descr.c

libcrypt_la_LIBADD=@GWEN_CRYPTO@/libcryptimpl.la


types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

