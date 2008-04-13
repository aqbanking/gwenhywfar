EXTRA_DIST=README ressource.rc.in

INCLUDES= -I$(top_builddir)/gwenhywfar3 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@


dbioplugindir = $(gwenhywfar_plugindir)/dbio

noinst_LTLIBRARIES=libdbio_csv.la
noinst_HEADERS=csv_p.h
libdbio_csv_la_SOURCES=csv.c

dbioplugin_LTLIBRARIES=csv.la
dbioplugin_DATA=csv.xml

csv_la_SOURCES=dummy.c
csv_la_LIBADD=libdbio_csv.la $(top_builddir)/src/libgwenhywfar.la
csv_la_DEPENDENCIES=libdbio_csv.la $(top_builddir)/src/libgwenhywfar.la

# if IS_WINDOWS
if IS_WINDOWS
csv_la_DEPENDENCIES += ressource.o
ressource_ldflags=-Wl,ressource.o
endif
# IS_WINDOWS

csv_la_LDFLAGS = -no-undefined $(STRIPALL) $(ressource_ldflags) \
    -module -version-info 0:1:0


sources:
	for f in $(csv_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

ressource.o: ressource.rc
	$(RC) -o $@ $<

