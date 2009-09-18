EXTRA_DIST=ressource.rc.in README

INCLUDES= -I$(top_builddir)/gwenhywfar3 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@

dbioplugindir = $(gwenhywfar_plugindir)/dbio

noinst_HEADERS=xmldb_p.h


dbioplugin_LTLIBRARIES=xmldb.la
dbioplugin_DATA=xmldb.xml

xmldb_la_SOURCES=xmldb.c
xmldb_la_LIBADD=$(top_builddir)/src/libgwenhywfar.la

xmldb_la_LDFLAGS = -no-undefined $(STRIPALL) \
    -module -version-info 0:1:0

sources:
	for f in $(libdbio_xmldb_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

ressource.o: ressource.rc
	$(RC) -o $@ $<


