SUBDIRS=dbio ct

INCLUDES= -I$(top_builddir)/gwenhywfar3

noinst_LTLIBRARIES=libplugins.la
libplugins_la_SOURCES=gwen_plugins.c
libplugins_la_LIBADD=ct/libct.la dbio/libdbio.la


sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

