INCLUDES=-I$(top_builddir)/gwenhywfar3

noinst_LTLIBRARIES=libfxhtml.la
libfxhtml_la_LIBADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)
libfxhtml_la_SOURCES=\
 g_box.c \
 g_generic.c \
 g_table.c \
 g_tablerow.c \
 g_unorderedlist.c \
 htmlctx.c \
 htmlfont.c \
 htmlgroup.c \
 htmlobject.c \
 htmlprops.c \
 o_box.c \
 o_grid.c \
 o_gridentry.c \
 o_word.c

#iheaderdir=${includedir}/gwen-gui-fox16/html
#iheader_HEADERS=

noinst_HEADERS= \
 g_box_p.h \
 g_generic_l.h \
 g_generic_p.h \
 g_table_l.h \
 g_table_p.h \
 g_tablerow_l.h \
 g_tablerow_p.h \
 g_unorderedlist_l.h \
 g_unorderedlist_p.h \
 htmlctx_l.h \
 htmlctx_p.h \
 htmlfont_p.h \
 htmlgroup_l.h \
 htmlgroup_p.h \
 htmlobject_p.h \
 htmlprops_p.h \
 o_box_l.h \
 o_grid_l.h \
 o_grid_p.h \
 o_gridentry_l.h \
 o_gridentry_p.h \
 o_word_l.h \
 htmlctx_be.h \
 htmlfont_be.h  \
 htmlobject_be.h \
 htmlprops_be.h


noinst_PROGRAMS=libtest
libtest_SOURCES=libtest.c
libtest_LDADD=libfxhtml.la

