SUBDIRS=dbio ct configmgr

INCLUDES= -I$(top_builddir)/gwenhywfar3

noinst_LTLIBRARIES=libgwen_plugins.la
libgwen_plugins_la_SOURCES=gwen_plugins.c
libgwen_plugins_la_LIBADD=ct/libct.la dbio/libdbio.la

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=gwen_plugins.h


sources:
	for f in $(libgwen_plugins_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

