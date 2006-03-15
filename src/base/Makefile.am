INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

# Don't include src/os above because its process.h conflicts with the
# system's <process.h>, at least for gcc3.2 on mingw32
# -I$(top_srcdir)/src/os 

AM_CFLAGS=@PROFILING_CFLAGS@ @visibility_cflags@  -DBUILDING_GWENHYWFAR

noinst_LTLIBRARIES=libbase.la

noinst_HEADERS=\
 args_p.h \
 buffer_p.h \
 debug_p.h \
 error_l.h \
 error_p.h \
 i18n_l.h \
 idlist_p.h \
 inherit_p.h \
 list_p.h \
 list1_p.h \
 logger_l.h \
 logger_p.h \
 memory_l.h \
 memory_p.h \
 pathmanager_l.h \
 pathmanager_p.h \
 plugindescr_p.h \
 plugin_l.h \
 plugin_p.h \
 refptr_p.h \
 ringbuffer_p.h \
 stringlist_p.h \
 stringlist2_p.h \
 waitcallback_l.h \
 waitcallback_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 args.h \
 buffer.h \
 debug.h \
 error.h \
 i18n.h \
 idlist.h \
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
 waitcallback.h

libbase_la_SOURCES=\
 args.c \
 buffer.c \
 debug.c \
 error.c \
 i18n.c \
 idlist.c \
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
 waitcallback.c


sources:
	for f in $(libbase_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

