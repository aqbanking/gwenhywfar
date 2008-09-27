DIST_SUBDIRS=dir
SUBDIRS=@gwenhywfar_plugins_cfgmgr_dirs@


noinst_LTLIBRARIES=libcfgmgr.la
libcfgmgr_la_SOURCES=dummy.c
libcfgmgr_la_LIBADD=$(gwenhywfar_plugins_cfgmgr_libs)


sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

