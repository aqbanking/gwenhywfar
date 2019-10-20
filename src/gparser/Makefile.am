INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ @WERROR@ -DBUILDING_GWENHYWFAR

typefiles=gwen_parser_element.xml

noinst_LTLIBRARIES=libgparser.la


typedatadir=$(datadir)/gwenhywfar/typemaker2/c
dist_typedata_DATA=\
  gwen_parser_element.tm2 gwen_parser_element_list.tm2 gwen_parser_element_list2.tm2 gwen_parser_element_tree.tm2


built_sources= \
  gwen_parser_element.c


build_headers_priv= \
  gwen_parser_element_p.h


build_headers_pub= \
  gwen_parser_element.h


build_headers=$(build_headers_priv) $(build_headers_pub)

libgparser_la_SOURCES=$(built_sources) \
  parser_xml.c \
  gwen_parser.c


iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=$(build_headers_pub) \
  parser_xml.h \
  gwen_parser.h


noinst_HEADERS=$(build_headers_priv)


EXTRA_DIST=$(typefiles) $(build_headers) $(built_sources) \
  gwen_parser_check.c \
  gwen_parser_update.c




build_sources: $(built_sources)
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d build_sources; \
	done ;

clean_build_files:
	rm -f $(built_sources) $(build_headers)

format_build_files:
	for f in $(built_sources) $(build_headers); do \
	  $(ASTYLE) -s2 --min-conditional-indent=0 --indent-labels --pad=oper -a --suffix=none $$f; \
	done


%.c: %.xml
	typemaker2 -I $(top_srcdir)/src/lib/typemaker2/c -I $(srcdir) -I $(AQDATABASE_TYPEMAKERDIR)/c --destfolder="$(srcdir)" -D GWENHYWFAR_API  $<

%.h: %.xml
	typemaker2 -I $(top_srcdir)/src/lib/typemaker2/c -I $(srcdir) -I $(AQDATABASE_TYPEMAKERDIR)/c --destfolder="$(srcdir)" -D GWENHYWFAR_API  $<


SUFFIXES = .xml .c


types: $(typefiles) typedefs
	typemaker2 -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)

typedefs: $(typefiles)
	typemaker2 --defs-only -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)



sources:
	for f in $(libgparser_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


