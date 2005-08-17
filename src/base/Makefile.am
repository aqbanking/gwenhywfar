INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui

# Don't include src/os above because its process.h conflicts with the
# system's <process.h>, at least for gcc3.2 on mingw32
# -I$(top_srcdir)/src/os 

localedir=$(datadir)/locale
DEFS=-DLOCALEDIR=\"$(localedir)\" @DEFS@


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

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 args.h \
 buffer.h \
 debug.h \
 error.h \
 i18n.h \
 idlist.h \
 inherit.h \
 list.h \
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

