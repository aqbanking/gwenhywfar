EXTRA_DIST = ressource.rc.in

INCLUDES= -I$(top_builddir)/gwenhywfar3 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser

AM_CFLAGS=@PROFILING_CFLAGS@

crypttokenplugindir = $(gwenhywfar_plugindir)/ct

crypttokenplugin_LTLIBRARIES=ohbci.la
crypttokenplugin_DATA=ohbci.xml
noinst_HEADERS=ohbci_l.h ohbci_p.h

ohbci_la_SOURCES=ohbci.c
ohbci_la_LIBADD=$(top_builddir)/src/libgwenhywfar.la
ohbci_la_DEPENDENCIES=$(top_builddir)/src/libgwenhywfar.la

# if IS_WINDOWS
if IS_WINDOWS
ohbci_la_DEPENDENCIES += ressource.o
ressource_ldflags=-Wl,ressource.o
endif
# IS_WINDOWS

ohbci_la_LDFLAGS = -no-undefined $(STRIPALL) $(ressource_ldflags) \
  -module -omit-version

sources:
	for f in $(ohbci_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

ressource.o: ressource.rc
	$(RC) -o $@ $<



