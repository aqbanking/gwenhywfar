INCLUDES=@all_includes@

SUBDIRS=@GWEN_CRYPTO@ none

EXTRA_DIST=\
 ossl/cryptssl_p.h \
 ossl/cryptssl.c \
 none/cryptnone_p.h \
 none/cryptnone.c
 
iheaderdir=${includedir}/gwenhyfwar
iheader_HEADERS=\
 crypt.h

noinst_HEADERS=\
 crypt_p.h

noinst_LTLIBRARIES=libcrypt.la

libcrypt_la_SOURCES=\
 crypt.c

libcrypt_la_LIBADD=@GWEN_CRYPTO@/libcryptimpl.la
