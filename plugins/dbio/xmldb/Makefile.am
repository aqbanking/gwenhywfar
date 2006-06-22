EXTRA_DIST=ressource.rc.in README

INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@

dbioplugindir = $(gwenhywfar_plugindir)/dbio


dbioplugin_LTLIBRARIES=xmldb.la
dbioplugin_DATA=xmldb.xml
noinst_HEADERS=xmldb_p.h

xmldb_la_SOURCES=xmldb.c
xmldb_la_LIBADD=$(top_builddir)/src/libgwenhywfar.la
xmldb_la_DEPENDENCIES=$(top_builddir)/src/libgwenhywfar.la

# if IS_WINDOWS
if IS_WINDOWS
xmldb_la_DEPENDENCIES += ressource.o
ressource_ldflags=-Wl,ressource.o
endif
# IS_WINDOWS

xmldb_la_LDFLAGS = -no-undefined $(STRIPALL) $(ressource_ldflags) \
    -module -version-info 0:1:0

sources:
	for f in $(xmldb_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

ressource.o: ressource.rc
	$(RC) -o $@ $<


