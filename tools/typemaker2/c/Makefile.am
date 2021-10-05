
INCLUDES=@all_includes@ -I$(top_builddir) -I../ -I../../ -I$(top_srcdir)/tools -I$(gwenhywfar_symlinkdir)


noinst_LTLIBRARIES=libtm2_c.la


libtm2_c_la_SOURCES=\
 tm_builder_c.c \
 tm2c_misc.c \
 tm2c_constructor.c \
 tm2c_destructor.c \
 tm2c_dup.c \
 tm2c_copy.c \
 tm2c_typedef.c \
 tm2c_struct.c \
 tm2c_getter.c \
 tm2c_setter.c \
 tm2c_readdb.c \
 tm2c_writedb.c \
 tm2c_readxml.c \
 tm2c_writexml.c \
 tm2c_readobject.c \
 tm2c_writeobject.c
 


noinst_HEADERS=\
 tm_builder_c.h \
 tm2c_misc.h \
 tm2c_constructor.h \
 tm2c_destructor.h \
 tm2c_dup.h \
 tm2c_copy.h \
 tm2c_typedef.h \
 tm2c_struct.h \
 tm2c_getter.h \
 tm2c_setter.h \
 tm2c_readdb.h \
 tm2c_writedb.h \
 tm2c_readxml.h \
 tm2c_writexml.h \
 tm2c_readobject.h \
 tm2c_writeobject.h



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



