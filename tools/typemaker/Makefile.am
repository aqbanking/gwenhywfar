INCLUDES=@all_includes@ -I$(gwenhywfar_symlinkdir)

EXTRA_DIST=test.xml

bin_PROGRAMS=typemaker
noinst_HEADERS=args.h typemaker_p.h
typemaker_SOURCES=args.c main.c phc.c lhc.c ahc.c code_c.c


if IS_WINDOWS

typemaker_LDFLAGS=-L$(top_builddir)/src/.libs -mconsole
typemaker_LDADD= -lgwenhywfar-$(GWENHYWFAR_SO_EFFECTIVE) $(PROG_LIBRARIES)

else

typemaker_LDADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)

endif


sources:
	for f in $(typemaker_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

