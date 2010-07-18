INCLUDES=-I$(gwenhywfar_symlinkdir)

AM_CPPFLAGS= @visibility_cflags@
AM_CFLAGS=@visibility_cflags@ 
DEFS+=-DPACKAGE=\"$(PACKAGE)\" -DBUILDING_GWENHYWFAR


noinst_LTLIBRARIES=libgwenhtml.la
libgwenhtml_la_SOURCES=\
 g_box.c \
 g_generic.c \
 g_table.c \
 g_tablerow.c \
 g_unorderedlist.c \
 htmlctx.c \
 htmlfont.c \
 htmlimage.c \
 htmlgroup.c \
 htmlobject.c \
 htmlprops.c \
 o_box.c \
 o_grid.c \
 o_gridentry.c \
 o_image.c \
 o_word.c

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS= \
 htmlctx_be.h \
 htmlfont_be.h  \
 htmlimage_be.h \
 htmlobject_be.h \
 htmlprops_be.h \
 o_image_be.h

noinst_HEADERS= \
 g_box_l.h \
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
 htmlimage_p.h \
 htmlgroup_l.h \
 htmlgroup_p.h \
 htmlobject_p.h \
 htmlprops_p.h \
 o_box_l.h \
 o_grid_l.h \
 o_grid_p.h \
 o_gridentry_l.h \
 o_gridentry_p.h \
 o_image_p.h \
 o_word_l.h


#noinst_PROGRAMS=libtest
#libtest_SOURCES=libtest.c
#libtest_LDADD=libgwenhtml.la

