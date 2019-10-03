AM_CPPFLAGS=-I$(gwenhywfar_symlinkdir) $(fox_includes) -Ihtml \
  @visibility_cflags@
DEFS+=-DBUILDING_CPP_GUI -DPACKAGE=\"$(PACKAGE)\"

lib_LTLIBRARIES=libgwengui-cpp.la

cmakeconfigdir = $(libdir)/cmake/gwengui-cpp-@GWENHYWFAR_VERSION_MAJOR@.@GWENHYWFAR_VERSION_MINOR@
cmakeconfig_DATA = gwengui-cpp-config.cmake gwengui-cpp-config-version.cmake

noinst_HEADERS=\
  cppdialog_p.hpp \
  cppgui_p.hpp \
  cppwidget_p.hpp

libgwengui_cpp_la_LIBADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)
libgwengui_cpp_la_LDFLAGS=-no-undefined

libgwengui_cpp_la_SOURCES=\
  cppdialog.cpp \
  cppgui.cpp \
  cppwidget.cpp

iheaderdir=${gwenhywfar_headerdir_am}/gwen-gui-cpp
iheader_HEADERS=\
 api.h \
 cppgui.hpp \
 cppdialog.hpp \
 cppwidget.hpp

