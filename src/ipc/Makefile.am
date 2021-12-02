INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ @WERROR@ -DBUILDING_GWENHYWFAR

typefiles=

noinst_LTLIBRARIES=libgwenipc.la


typedatadir=$(datadir)/gwenhywfar/typemaker2/c
dist_typedata_DATA=


built_sources=


build_headers_priv=


build_headers_pub=


build_headers=$(build_headers_priv) $(build_headers_pub)

libgwenipc_la_SOURCES=$(built_sources) \
  pdu_item.c \
  ipc_proto.c

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=$(build_headers_pub) \
  pdu_item.h \
  ipc_proto.h


noinst_HEADERS=$(build_headers_priv) \
  pdu_item_p.h \
  ipc_proto_p.h


EXTRA_DIST=$(typefiles) $(build_headers) $(built_sources) \
  pdu_item-t.c





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
	for f in $(libgwenipc_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


