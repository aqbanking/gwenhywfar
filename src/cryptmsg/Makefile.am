INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser $(LIBGCRYPT_CFLAGS) @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 sighead.h \
 sigtail.h \
 signedobject.h

noinst_LTLIBRARIES=libcryptmsg.la

noinst_HEADERS=\
 sighead_p.h \
 sigtail_p.h \
 signedobject_p.h


libcryptmsg_la_SOURCES=\
 sighead.c \
 sigtail.c \
 signedobject.c


types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

sources:
	for f in $(libcryptmsg_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


