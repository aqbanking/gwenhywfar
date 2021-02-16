AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/sio \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ @WERROR@ -DBUILDING_GWENHYWFAR

typefiles=\
  gwen_sar_fileheader.xml
 
built_sources= \
  gwen_sar_fileheader.c

build_headers_priv= \
  gwen_sar_fileheader_l.h \
  gwen_sar_fileheader_p.h
 

build_headers_pub=\
  gwen_sar_fileheader.h

build_headers=$(build_headers_priv) $(build_headers_pub)


noinst_LTLIBRARIES=libsar.la


noinst_HEADERS=$(build_headers_priv)\
  sar_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=$(build_headers_pub)\
  sar.h

libsar_la_SOURCES=$(built_sources)\
  sar.c


EXTRA_DIST=$(typefiles) $(built_sources) $(build_headers)




build_sources: $(built_sources)
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d build_sources; \
	done ;


sources:
	for f in $(libsar_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


types: $(typefiles)
	typemaker2 -I $(top_srcdir)/tools/typemaker2/types/c -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)


typedefs: $(typefiles)
	typemaker2 --defs-only -I $(top_srcdir)/tools/typemaker2/types/c -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)


clean_build_files:
	rm -f $(built_sources) $(build_headers)

format_build_files:
	for f in $(built_sources) $(build_headers); do \
	  $(ASTYLE) -s2 --min-conditional-indent=0 --indent-labels --pad=oper -a --suffix=none $$f; \
	done


