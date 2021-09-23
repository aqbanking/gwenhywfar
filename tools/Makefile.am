DIST_SUBDIRS=xmlmerge typemaker typemaker2 gcttool gsa gwenbuild
SUBDIRS=xmlmerge typemaker typemaker2 gcttool gsa @gwenbuild_tool@

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done
