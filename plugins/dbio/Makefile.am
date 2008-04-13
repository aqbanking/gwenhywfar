DIST_SUBDIRS=csv olddb xmldb
SUBDIRS=@gwenhywfar_plugins_dbio_dirs@

EXTRA_DIST=README


noinst_LTLIBRARIES=libdbio.la
libdbio_la_SOURCES=dummy.c
libdbio_la_LIBADD=$(gwenhywfar_plugins_dbio_libs)


sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

