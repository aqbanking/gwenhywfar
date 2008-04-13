SUBDIRS=dbio ct

INCLUDES= -I$(top_builddir)/gwenhywfar3

noinst_LTLIBRARIES=libgwen_plugins.la
libgwen_plugins_la_SOURCES=gwen_plugins.c
libgwen_plugins_la_LIBADD=ct/libct.la dbio/libdbio.la


sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

