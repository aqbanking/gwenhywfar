INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@PROFILING_CFLAGS@

certdir=$(sysconfdir)
dist_cert_DATA=gwen-public-ca.crt

noinst_LTLIBRARIES=libcryptimpl.la


noinst_HEADERS=\
 bio_netlayer_l.h \
 bio_netlayer_p.h \
 cryptssl_l.h \
 cryptssl_p.h \
 cryptssl_bf_p.h \
 cryptssl_des_p.h \
 cryptssl_md5_p.h \
 cryptssl_sha1_p.h \
 cryptssl_rmd160_p.h \
 cryptssl_rsa_p.h \
 nl_ssl_p.h

libcryptimpl_la_SOURCES=\
 cryptssl_bf.c \
 cryptssl_des.c \
 cryptssl_md5.c \
 cryptssl_sha1.c \
 cryptssl_rmd160.c \
 cryptssl_rsa.c \
 cryptssl.c \
 bio_netlayer.c \
 nl_ssl.c


sources:
	for f in $(libcryptimpl_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done
 



