INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui

SUBDIRS=@GWEN_CRYPTO@
DIST_SUBDIRS=none ossl

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 crypt.h \
 keymanager.h \
 keyspec.h \
 md.h \
 padd.h \
 nettransportssl.h

noinst_HEADERS=\
 crypt_l.h \
 crypt_p.h \
 keymanager_p.h \
 keyspec_p.h \
 md_p.h \
 padd_p.h

noinst_LTLIBRARIES=libcrypt.la

libcrypt_la_SOURCES=\
 crypt.c \
 keymanager.c \
 keyspec.c \
 md.c \
 padd.c

libcrypt_la_LIBADD=@GWEN_CRYPTO@/libcryptimpl.la
