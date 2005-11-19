SUBDIRS=csv olddb
EXTRA_DIST=README

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done
