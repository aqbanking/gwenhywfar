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


noinst_LTLIBRARIES=libbase.la

noinst_HEADERS=\
 args_p.h \
 buffer_p.h \
 debug_p.h \
 error_l.h \
 error_p.h \
 idlist_p.h \
 inherit_p.h \
 list_p.h \
 logger_p.h \
 memory_l.h \
 memory_p.h \
 plugindescr_p.h \
 ringbuffer_p.h \
 stringlist_p.h \
 waitcallback_l.h \
 waitcallback_p.h

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
 args.h \
 buffer.h \
 debug.h \
 error.h \
 idlist.h \
 inherit.h \
 list.h \
 list2.h \
 logger.h \
 memory.h \
 misc.h \
 misc2.h \
 plugindescr.h \
 ringbuffer.h \
 stringlist.h \
 waitcallback.h

libbase_la_SOURCES=\
 args.c \
 buffer.c \
 debug.c \
 error.c \
 idlist.c \
 inherit.c \
 list.c \
 logger.c \
 memory.c \
 plugindescr.c \
 ringbuffer.c \
 stringlist.c \
 waitcallback.c

