AM_CPPFLAGS=-I$(gwenhywfar_symlinkdir) $(GTK3_CFLAGS) \
  @visibility_cflags@
DEFS+=-DBUILDING_GTK3_GUI -DPACKAGE=\"$(PACKAGE)\"

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwengui-gtk3.pc

lib_LTLIBRARIES=libgwengui-gtk3.la


libgwengui_gtk3_la_LIBADD=$(GTK3_LIBS) $(top_builddir)/src/$(gwenhywfar_internal_libname)
libgwengui_gtk3_la_LDFLAGS = -no-undefined -version-info \
   $(GWENHYWFAR_SO_CURRENT):$(GWENHYWFAR_SO_REVISION):$(GWENHYWFAR_SO_AGE)

noinst_HEADERS=\
  gtk3_gui_p.h \
  gtk3_gui_dialog_l.h \
  gtk3_gui_dialog_p.h

libgwengui_gtk3_la_SOURCES=\
  gtk3_gui_dialog.c \
  gtk3_gui.c


EXTRA_DIST=\
  w_combobox.c \
  w_label.c \
  w_dialog.c \
  w_gridlayout.c \
  w_groupbox.c \
  w_hlayout.c \
  w_hline.c \
  w_hspacer.c \
  w_lineedit.c \
  w_vlayout.c \
  w_vline.c \
  w_vspacer.c \
  w_pushbutton.c \
  w_textbrowser.c \
  w_textedit.c \
  w_stack.c \
  w_tabbook.c \
  w_progressbar.c \
  w_listbox.c \
  w_checkbox.c \
  w_scrollarea.c \
  w_radiobutton.c \
  w_spinbox.c \
  w_vsplitter.c \
  w_hsplitter.c



iheaderdir=${gwenhywfar_headerdir_am}/gwen-gui-gtk3
iheader_HEADERS= \
  gtk3_gui.h


noinst_PROGRAMS = libtest
libtest_SOURCES = libtest.c
libtest_LDADD = libgwengui-gtk3.la $(GTK3_LIBS) $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(builddir)/../testdialogs/libgwengui-test.la

