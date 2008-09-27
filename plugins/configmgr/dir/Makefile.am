EXTRA_DIST=README dir.xml.in

INCLUDES= -I$(top_builddir)/gwenhywfar3 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@

cfgmgrplugindir = $(gwenhywfar_plugindir)/configmgr

noinst_LTLIBRARIES=libcfgmgr_dir.la
libcfgmgr_dir_la_SOURCES=cfgdir.c
noinst_HEADERS=cfgdir_p.h


cfgmgrplugin_LTLIBRARIES=dir.la
cfgmgrplugin_DATA=dir.xml

dir_la_SOURCES=dummy.c
dir_la_LIBADD=libcfgmgr_dir.la $(top_builddir)/src/libgwenhywfar.la
dir_la_DEPENDENCIES=libcfgmgr_dir.la $(top_builddir)/src/libgwenhywfar.la

dir_la_LDFLAGS = -no-undefined $(STRIPALL) $(ressource_ldflags) \
    -module -avoid-version

sources:
	for f in $(libcfgmgr_dir_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

ressource.o: ressource.rc
	$(RC) -o $@ $<


