AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir) \
 -I$(top_srcdir)/src/ \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/gui \
 -I$(top_srcdir)/src/parser

# Don't include src/os above because its process.h conflicts with the
# system's <process.h>, at least for gcc3.2 on mingw32
# -I$(top_srcdir)/src/os 

AM_CFLAGS=@PROFILING_CFLAGS@ @visibility_cflags@ @WERROR@  -DBUILDING_GWENHYWFAR

DEFS+=-DGWEN_BUFFER_DEFAULT_HARDLIMIT='@gwenhywfar_buffer_hardlimit@'

noinst_LTLIBRARIES=libbase.la


typedatadir=$(datadir)/gwenhywfar/typemaker2/c
dist_typedata_DATA=\
  gwen_param.tm2 gwen_param_list.tm2 gwen_param_list2.tm2 gwen_param_tree.tm2


typefiles=param.xml


EXTRA_DIST=$(typefiles) \
  param_fns.c \
  simpleptrlist-t.c \
  idlist64-t.c



noinst_HEADERS=\
 args_p.h \
 buffer_p.h \
 debug_p.h \
 error_l.h \
 error_p.h \
 gwensignal_p.h \
 i18n_l.h \
 idlist64_p.h \
 inherit_p.h \
 list_p.h \
 list1_p.h \
 logger_l.h \
 logger_p.h \
 pathmanager_l.h \
 pathmanager_p.h \
 plugindescr_p.h \
 plugin_l.h \
 plugin_p.h \
 refptr_p.h \
 ringbuffer_p.h \
 stringlist_p.h \
 stringlist2_p.h \
 idmap_p.h \
 memcache_p.h \
 tree_p.h \
 tree2_p.h \
 simpleptrlist_p.h \
 multicache_p.h \
 param_p.h


iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 args.h \
 buffer.h \
 debug.h \
 error.h \
 gwensignal.h \
 i18n.h \
 idlist64.h \
 inherit.h \
 list.h \
 list1.h \
 list1.tmpl \
 list2.h \
 list2.tmpl \
 logger.h \
 memory.h \
 misc.h \
 misc2.h \
 pathmanager.h \
 plugin.h \
 plugindescr.h \
 refptr.h \
 ringbuffer.h \
 stringlist.h \
 stringlist2.h \
 idmap.h \
 fastbuffer.h \
 memcache.h \
 tree.h \
 tree2.h \
 simpleptrlist.h \
 multicache.h \
 bindata.h \
 param.h \
 param_fns.h \
 endianfns.h


libbase_la_SOURCES=\
 args.c \
 buffer.c \
 debug.c \
 error.c \
 gwensignal.c \
 i18n.c \
 idlist64.c \
 inherit.c \
 list.c \
 list1.c \
 logger.c \
 memory.c \
 pathmanager.c \
 plugin.c \
 plugindescr.c \
 refptr.c \
 ringbuffer.c \
 stringlist.c \
 stringlist2.c \
 idmap.c \
 fastbuffer.c \
 memcache.c \
 tree.c \
 tree2.c \
 simpleptrlist.c \
 multicache.c \
 param.c


sources:
	for f in $(libbase_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


types: $(typefiles) typedefs
	typemaker2 -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)

typedefs: $(typefiles)
	typemaker2 --defs-only -I $(srcdir) --destfolder="$(srcdir)" -D GWENHYWFAR_API $(typefiles)



splint:
	for f in $(libbase_la_SOURCES); do \
	  splint $(AM_CPPFLAGS) $(DEFS) +enumint -unrecogcomments +posixlib -predboolint +boolint -boolops $$f ; \
	done


