INCLUDES=@all_includes@

SUBDIRS=@GWEN_CRYPTO@

EXTRA_DIST=\
 ossl/Makefile.am \
 ossl/Makefile.in \
 ossl/cryptssl_p.h \
 ossl/cryptssl.c \
 none/Makefile.am \
 none/Makefile.in \
 none/cryptnone_p.h \
 none/cryptnone.c
 
iheaderdir=${includedir}/gwenhyfwar
iheader_HEADERS=\
 crypt.h \
 keymanager.h \
 keyspec.h \
 md.h

noinst_HEADERS=\
 crypt_p.h \
 keymanager_p.h \
 keyspec_p.h \
 md_p.h

noinst_LTLIBRARIES=libcrypt.la

libcrypt_la_SOURCES=\
 crypt.c \
 keymanager.c \
 keyspec.c \
 md.c

libcrypt_la_LIBADD=@GWEN_CRYPTO@/libcryptimpl.la
