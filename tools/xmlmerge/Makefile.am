INCLUDES=@all_includes@ -I$(gwenhywfar_symlinkdir)

bin_PROGRAMS=xmlmerge
noinst_HEADERS=args.h
xmlmerge_SOURCES=args.c main.c

if IS_WINDOWS

xmlmerge_LDFLAGS=-L$(top_builddir)/src/.libs -mconsole
xmlmerge_LDADD= -lgwenhywfar-$(GWENHYWFAR_SO_EFFECTIVE) $(PROG_LIBRARIES)

else

xmlmerge_LDADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)

endif

sources:
	for f in $(xmlmerge_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


