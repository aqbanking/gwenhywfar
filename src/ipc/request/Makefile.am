INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

typefiles=request.xml
EXTRA_DIST=$(typefiles)


noinst_LTLIBRARIES=libipcrequest.la

noinst_HEADERS=\
  request_l.h \
  request_p.h \
  requestmgr_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
  request.h \
  requestmgr.h

libipcrequest_la_SOURCES=\
  request.c \
  requestmgr.c


types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

sources:
	for f in $(libipcrequest_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

