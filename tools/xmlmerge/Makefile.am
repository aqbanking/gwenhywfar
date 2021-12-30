AM_CPPFLAGS=@all_includes@ -I$(gwenhywfar_symlinkdir)

bin_PROGRAMS=xmlmerge
noinst_HEADERS=args.h
xmlmerge_SOURCES=args.c main.c

xmlmerge_LDADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)
xmlmerge_LDFLAGS=@i18n_libs@

if IS_WINDOWS

xmlmerge_LDFLAGS+=-mconsole
xmlmerge_LDADD+=$(PROG_LIBRARIES)

endif

sources:
	for f in $(xmlmerge_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


