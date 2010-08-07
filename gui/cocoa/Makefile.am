INCLUDES=-I$(gwenhywfar_symlinkdir) $(COCOA_CFLAGS)

AM_CPPFLAGS= @visibility_cflags@
DEFS+=-DBUILDING_COCOA_GUI -DPACKAGE=\"$(PACKAGE)\"

lib_LTLIBRARIES=libgwengui-cocoa.la


libgwengui_cocoa_la_LIBADD=$(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(COCOA_FRAMEWORK) $(FOUNDATION_FRAMEWORK) $(APPKIT_FRAMEWORK) $(COREDATA_FRAMEWORK) \
  -lobjc

#libgwengui_cocoa_la_LDFLAGS=
noinst_HEADERS=\
  cocoa_gui_p.h

libgwengui_cocoa_la_SOURCES=\
  cocoa_gui.mm


iheaderdir=${includedir}/gwen-gui-cocoa
iheader_HEADERS= \
  cocoa_gui.h


noinst_PROGRAMS = libtest
libtest_SOURCES = libtest.m
#libtest_LDFLAGS=
libtest_LDADD = libgwengui-cocoa.la $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(COCOA_FRAMEWORK) $(FOUNDATION_FRAMEWORK) $(APPKIT_FRAMEWORK) $(COREDATA_FRAMEWORK) \
  -lobjc \
  $(builddir)/../testdialogs/libgwengui-test.la

