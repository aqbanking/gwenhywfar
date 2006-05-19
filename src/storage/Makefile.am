INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR


typefiles=st_vardef.xml st_log.xml
EXTRA_DIST=$(typefiles)


noinst_LTLIBRARIES=libsto.la
iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 st_client.h st_client_be.h \
 st_find.h st_find_be.h \
 st_object.h st_object_be.h \
 st_storage.h st_storage_be.h \
 st_type.h st_type_be.h \
 st_vardef.h \
 st_log.h \
 st_plugin.h st_plugin_be.h \
 smp_type_be.h \
 smp_storage_be.h

noinst_HEADERS=\
 st_client_p.h \
 st_find_p.h \
 st_object_p.h \
 st_storage_l.h \
 st_storage_p.h \
 st_type_p.h \
 st_vardef_l.h st_vardef_p.h \
 st_log_l.h st_log_p.h \
 st_plugin_p.h \
 smp_find_l.h smp_find_p.h \
 smp_type_p.h \
 smp_storage_p.h

libsto_la_SOURCES=\
 st_client.c \
 st_find.c \
 st_object.c \
 st_storage.c \
 st_type.c \
 st_vardef.c \
 st_log.c \
 st_plugin.c \
 smp_find.c \
 smp_type.c \
 smp_storage.c


types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

sources:
	for f in $(libnet2_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


