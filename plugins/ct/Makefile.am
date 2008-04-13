DIST_SUBDIRS=ohbci
SUBDIRS=@gwenhywfar_plugins_ct_dirs@


noinst_LTLIBRARIES=libct.la
libct_la_SOURCES=dummy.c
libct_la_LIBADD=$(gwenhywfar_plugins_ct_libs)


sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

