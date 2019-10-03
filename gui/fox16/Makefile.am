AM_CPPFLAGS=-I$(gwenhywfar_symlinkdir) $(fox_includes) -Ihtml \
  @visibility_cflags@
DEFS+=-DBUILDING_FOX16_GUI -DPACKAGE=\"$(PACKAGE)\"

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwengui-fox16.pc


lib_LTLIBRARIES=libgwengui-fox16.la

noinst_HEADERS=\
  fox16_gui_dialog_l.hpp \
  fox16_gui_sortinglist_l.hpp \
  fox16_gui_updater_l.hpp \
  fox16_htmlctx_p.hpp \
  fox16_htmlctx.hpp

libgwengui_fox16_la_LIBADD=$(fox_libs) $(top_builddir)/src/$(gwenhywfar_internal_libname) $(top_builddir)/gui/cpp/libgwengui-cpp.la
libgwengui_fox16_la_LDFLAGS=-no-undefined

libgwengui_fox16_la_SOURCES=\
  fox16_gui.cpp \
  fox16_gui_dialog.cpp \
  fox16_gui_sortinglist.cpp \
  fox16_gui_updater.cpp \
  fox16_htmlctx.cpp \
  fox16_htmllabel.cpp \
  fox16_htmltext.cpp \
  theme.cpp

iheaderdir=${gwenhywfar_headerdir_am}/gwen-gui-fox16
iheader_HEADERS=\
 fox16_gui.hpp \
 fox16_htmlctx.hpp \
 fox16_htmllabel.hpp \
 fox16_htmltext.hpp \
 theme.h \
 api.h

noinst_PROGRAMS=libtest
libtest_SOURCES=libtest.cpp
libtest_LDADD=libgwengui-fox16.la $(fox_libs) $(top_builddir)/src/$(gwenhywfar_internal_libname) \
 $(top_builddir)/gui/cpp/libgwengui-cpp.la \
 $(builddir)/../testdialogs/libgwengui-test.la

