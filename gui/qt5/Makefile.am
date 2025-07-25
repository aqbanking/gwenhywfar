AM_CPPFLAGS= @visibility_cflags@ @QT_CXXFLAGS@ -I$(gwenhywfar_symlinkdir) $(QT_CXXFLAGS)
DEFS=-DBUILDING_QT5_GUI

qt5_moc      = @QT_MOC@
qt5_uic      = @QT_UIC@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwengui-qt@QT_MAJOR_VERSION@.pc

cmakeconfigdir = $(libdir)/cmake/gwengui-qt@QT_MAJOR_VERSION@-@GWENHYWFAR_VERSION_MAJOR@.@GWENHYWFAR_VERSION_MINOR@
cmakeconfig_DATA = gwengui-qt@QT_MAJOR_VERSION@-config.cmake gwengui-qt@QT_MAJOR_VERSION@-config-version.cmake

lib_LTLIBRARIES=libgwengui-qt@QT_MAJOR_VERSION@.la
noinst_PROGRAMS=libtest

noinst_HEADERS=

libgwengui_qt@QT_MAJOR_VERSION@_la_LIBADD=$(QT_LIBS) $(top_builddir)/src/$(gwenhywfar_internal_libname) $(builddir)/../cpp/libgwengui-cpp.la
libgwengui_qt@QT_MAJOR_VERSION@_la_LDFLAGS=-no-undefined -version-info \
   $(GWENHYWFAR_SO_CURRENT):$(GWENHYWFAR_SO_REVISION):$(GWENHYWFAR_SO_AGE)

libgwengui_qt@QT_MAJOR_VERSION@_la_SOURCES=\
  qt5dialogbox.cpp \
  qt5_gui.cpp \
  qt5_gui_dialog.cpp

nodist_libgwengui_qt@QT_MAJOR_VERSION@_la_SOURCES=

nodist_noinst_HEADERS=


iheaderdir=${gwenhywfar_headerdir_am}/gwen-gui-qt5
iheader_HEADERS=\
 qt5_gui.hpp \
 qt5_gui_dialog.hpp \
 qt5dialogbox.hpp

MOC_FILES=\
  qt5dialogbox.moc


UI_FILES=


EXTRA_DIST=$(UI_FILES) \
  w_widget.cpp \
  w_dialog.cpp \
  w_vlayout.cpp \
  w_hlayout.cpp \
  w_gridlayout.cpp \
  w_label.cpp \
  w_lineedit.cpp \
  w_pushbutton.cpp \
  w_hline.cpp \
  w_vline.cpp \
  w_textedit.cpp \
  w_combobox.cpp \
  w_tabbook.cpp \
  w_checkbox.cpp \
  w_groupbox.cpp \
  w_widgetstack.cpp \
  w_textbrowser.cpp \
  w_scrollarea.cpp \
  w_progressbar.cpp \
  w_listbox.cpp \
  w_radiobutton.cpp \
  w_spinbox.cpp

libtest_SOURCES=libtest.cpp
libtest_LDADD=libgwengui-qt@QT_MAJOR_VERSION@.la $(top_builddir)/src/$(gwenhywfar_internal_libname) $(QT_LIBS) \
  $(top_builddir)/gui/cpp/libgwengui-cpp.la \
  $(builddir)/../testdialogs/libgwengui-test.la

SUFFIXES = .ui .ui.hpp .ui.cpp .moc


%.moc: %.hpp
	$(qt5_moc) -o $@ $<

%.ui.hpp: %.ui
	$(qt5_uic) $< -o $@


sources:
	for f in $(libgwengui_qt@QT_MAJOR_VERSION@_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;


BUILT_SOURCES = $(MOC_FILES) $(nodist_libgwengui_qt@QT_MAJOR_VERSION@_la_SOURCES)
CLEANFILES = $(BUILT_SOURCES)
