INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt3 \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR


typefiles=\
  ct_context.xml \
  ct_keyinfo.xml


built_sources=  \
  ct_context.c \
  ct_keyinfo.c


build_headers_priv= \
  ct_context_p.h \
  ct_keyinfo_p.h


build_headers_pub= \
  ct_context.h \
  ct_keyinfo.h

build_headers=$(build_headers_priv) $(build_headers_pub)


iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS= $(build_headers_pub) \
  ct.h \
  ct_be.h \
  ctf_context_be.h \
  ctfile_be.h \
  ctplugin_be.h \
  ctplugin.h


noinst_LTLIBRARIES=libcrypttoken.la
noinst_HEADERS= $(build_headers_priv) \
  ct_p.h \
  ctf_context_p.h \
  ctfile_p.h \
  ctplugin_p.h \
  ctplugin_l.h


libcrypttoken_la_SOURCES= $(built_sources) \
  ct.c \
  ctf_context.c \
  ctfile.c \
  ctplugin.c

EXTRA_DIST=$(typefiles) $(built_sources) $(build_headers)


DEFS += -DLIBCHIPCARD_GWENHYWFAR_PLUGIN_PACKAGE=\"libchipcard-libgwenhywfar@GWENHYWFAR_SO_EFFECTIVE@-plugins\"

types: $(typefiles)
	typemaker2 -I $(top_srcdir)/tools/typemaker2/types/c -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)

typedefs: $(typefiles)
	typemaker2 --defs-only -I $(top_srcdir)/tools/typemaker2/types/c -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)


format_build_files:
	for f in $(built_sources) $(build_headers); do \
	  astyle --style=linux -s2 --min-conditional-indent=0 --indent-labels --pad-oper --suffix=none $$f; \
	done


sources:
	for f in $(libcrypttoken_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


