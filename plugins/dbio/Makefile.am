SUBDIRS=csv olddb rfc822 xmldb
EXTRA_DIST=README

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done
