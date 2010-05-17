INCLUDES=-I$(gwenhywfar_symlinkdir) $(GTK2_CFLAGS)

AM_CPPFLAGS= @visibility_cflags@
DEFS+=-DBUILDING_GTK2_GUI -DPACKAGE=\"$(PACKAGE)\"

lib_LTLIBRARIES=libgwengui-gtk2.la


libgwengui_gtk2_la_LIBADD=$(GTK2_LIBS) $(top_builddir)/src/$(gwenhywfar_internal_libname)
noinst_HEADERS=\
  gtk2_gui_dialog_p.h

libgwengui_gtk2_la_SOURCES=\
  gtk2_gui_dialog.c \
  gtk2_gui.c


EXTRA_DIST=\
  w_label.c \
  w_dialog.c \
  w_gridlayout.c \
  w_hlayout.c \
  w_hline.c \
  w_hspacer.c \
  w_lineedit.c \
  w_vlayout.c \
  w_vline.c \
  w_vspacer.c \
  w_pushbutton.c



iheaderdir=${includedir}/gwen-gui-gtk2
iheader_HEADERS= \
  gtk2_gui.h


noinst_PROGRAMS = libtest
libtest_SOURCES = libtest.c
libtest_LDADD = libgwengui-gtk2.la $(GTK2_LIBS) $(top_builddir)/src/$(gwenhywfar_internal_libname)

