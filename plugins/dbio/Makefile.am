DIST_SUBDIRS=csv olddb xmldb
SUBDIRS=@gwenhywfar_plugins_dbio_dirs@

EXTRA_DIST=README


sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

