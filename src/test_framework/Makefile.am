AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/parser \
 $(LIBGNUTLS_CFLAGS) \
 @all_includes@

AM_CFLAGS=@visibility_cflags@ @WERROR@ -DBUILDING_GWENHYWFAR

typefiles=\
  testmodule.xml

EXTRA_DIST=$(typefiles)

noinst_LTLIBRARIES=libtestframework.la

noinst_HEADERS=\
  testmodule_p.h \
  testframework_p.h \
  testgui_l.h \
  testgui_p.h


iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
  testmodule.h \
  testframework.h


libtestframework_la_SOURCES=\
  testmodule.c \
  testframework.c \
  testgui.c



types: $(typefiles)
	typemaker2 -I $(top_srcdir)/tools/typemaker2/types/c -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)

typedefs: $(typefiles)
	typemaker2 --defs-only -I $(top_srcdir)/tools/typemaker2/types/c -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)


format_build_files:
	for f in $(built_sources) $(build_headers); do \
	  $(ASTYLE) --style=linux -s2 --min-conditional-indent=0 --indent-labels --pad-oper --suffix=none $$f; \
	done


sources:
	for f in $(libtestframework_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

