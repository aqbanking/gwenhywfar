INCLUDES=@all_includes@

SUBDIRS=@GWEN_CRYPTO@
DIST_SUBDIRS=none ossl

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 crypt.h \
 keymanager.h \
 keyspec.h \
 md.h \
 padd.h \
 sslconnection.h \
 nettransportssl.h

noinst_HEADERS=\
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
