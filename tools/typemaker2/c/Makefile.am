
INCLUDES=@all_includes@ -I$(top_builddir) -I../ -I../../ -I$(top_srcdir)/tools -I$(gwenhywfar_symlinkdir)


noinst_LTLIBRARIES=libtm2_c.la


libtm2_c_la_SOURCES=\
 tm_builder_c.c \
 tm2c_constructor.c \
 tm2c_destructor.c \
 tm2c_typedef.c \
 tm2c_struct.c \
 tm2c_getter.c


noinst_HEADERS=\
 tm_builder_c.h \
 tm2c_constructor.h \
 tm2c_destructor.h \
 tm2c_typedef.h \
 tm2c_struct.h \
 tm2c_getter.h



sources:
	for f in $(typemaker2_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

indent:  $(BUILT_SOURCES)
	$(ASTYLE) \
	  --style=gnu \
	  --indent=spaces=2 \
	  --min-conditional-indent=0 \
	  --indent-labels \
	  --pad=oper \
	  --one-line=keep-blocks \
	  --suffix=none \
	  -a -y $(BUILT_SOURCES)

types: $(typefiles)
	./typemaker2 -I types/c -I $(srcdir) $(typefiles)



