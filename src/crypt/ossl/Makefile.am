INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui
 

noinst_LTLIBRARIES=libcryptimpl.la


noinst_HEADERS=\
 cryptssl_p.h \
 cryptssl_des_p.h \
 cryptssl_md5_p.h \
 cryptssl_rmd160_p.h \
 cryptssl_rsa_p.h \
 nettransportssl_p.h

libcryptimpl_la_SOURCES=\
 cryptssl_des.c \
 cryptssl_md5.c \
 cryptssl_rmd160.c \
 cryptssl_rsa.c \
 cryptssl.c \
 nettransportssl.c

