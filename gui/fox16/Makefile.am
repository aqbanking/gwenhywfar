INCLUDES=-I$(top_builddir)/gwenhywfar3 $(fox_includes)

AM_CPPFLAGS= @visibility_cflags@
DEFS=-DBUILDING_FOX16_GUI -DPACKAGE=\"$(PACKAGE)\"

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwengui-fox16.pc


lib_LTLIBRARIES=libgwengui-fox16.la

noinst_HEADERS=\
  cppdialog_p.hpp \
  cppgui_p.hpp \
  fox16_gui_dialog_l.hpp \
  fox16_gui_sortinglist_l.hpp \
  fox16_gui_updater_l.hpp

libgwengui_fox16_la_LIBADD=$(fox_libs) $(top_builddir)/src/$(gwenhywfar_internal_libname)
libgwengui_fox16_la_SOURCES=\
  cppdialog.cpp \
  cppgui.cpp \
  fox16_gui.cpp \
  fox16_gui_dialog.cpp \
  fox16_gui_sortinglist.cpp \
  fox16_gui_updater.cpp

iheaderdir=${includedir}/gwen-gui-fox16
iheader_HEADERS=\
 cppgui.hpp \
 cppdialog.hpp \
 fox16_gui.hpp

