SUBDIRS=dbio ct configmgr

AM_CPPFLAGS= -I$(top_builddir)/gwenhywfar3

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

