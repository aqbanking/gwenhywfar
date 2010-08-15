INCLUDES=-I$(gwenhywfar_symlinkdir) $(fox_includes) -Ihtml

DEFS+=-DPACKAGE=\"$(PACKAGE)\"

noinst_LTLIBRARIES=libgwengui-cpp.la

noinst_HEADERS=\
  cppdialog_p.hpp \
  cppgui_p.hpp \
  cppwidget_p.hpp

libgwengui_cpp_la_LIBADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)

libgwengui_cpp_la_SOURCES=\
  cppdialog.cpp \
  cppgui.cpp \
  cppwidget.cpp

iheaderdir=${gwenhywfar_headerdir_am}/gwen-gui-cpp
iheader_HEADERS=\
 cppgui.hpp \
 cppdialog.hpp \
 cppwidget.hpp

