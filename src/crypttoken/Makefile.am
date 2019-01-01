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


iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
  ct.h \
  ct_be.h \
  ct_context.h \
  ctf_context_be.h \
  ctfile_be.h \
  ct_keyinfo.h \
  ctplugin_be.h \
  ctplugin.h


noinst_LTLIBRARIES=libcrypttoken.la
noinst_HEADERS=\
  ct_p.h \
  ct_context_l.h \
  ct_context_p.h \
  ctf_context_p.h \
  ctfile_p.h \
  ct_keyinfo_p.h \
  ctplugin_p.h \
  ctplugin_l.h


libcrypttoken_la_SOURCES=\
  ct.c \
  ct_context.c \
  ctf_context.c \
  ctfile.c \
  ct_keyinfo.c \
  ctplugin.c

DEFS += -DLIBCHIPCARD_GWENHYWFAR_PLUGIN_PACKAGE=\"libchipcard-libgwenhywfar@GWENHYWFAR_SO_EFFECTIVE@-plugins\"

types: $(typefiles)
	typemaker2 -I $(top_srcdir)/tools/typemaker2/types/c -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)

sources:
	for f in $(libcrypttoken_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


