INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/parser $(LIBGCRYPT_CFLAGS) @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

#typefiles=crypttoken.xml


iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
  cryptalgo.h \
  cryptdefs.h \
  hashalgo.h \
  paddalgo.h \
  cryptkey.h \
  cryptkey_be.h \
  cryptkeyrsa.h \
  cryptkeysym.h \
  mdigest.h \
  mdigest_be.h \
  padd.h \
  smalltresor.h

noinst_LTLIBRARIES=libcrypt3.la
noinst_HEADERS=\
  cryptalgo_p.h \
  cryptdefs_p.h \
  hashalgo_p.h \
  paddalgo_p.h \
  cryptkey_l.h \
  cryptkey_p.h \
  cryptkeyrsa_p.h \
  cryptkeysym_p.h \
  mdigest_p.h \
  mdigestgc_p.h \
  padd_p.h 

libcrypt3_la_SOURCES=\
  cryptalgo.c \
  cryptdefs.c \
  hashalgo.c \
  paddalgo.c \
  cryptkey.c \
  cryptkeyrsa.c \
  cryptkeysym.c \
  mdigest.c \
  mdigestgc.c \
  padd.c \
  smalltresor.c


types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

sources:
	for f in $(libcrypt3_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

