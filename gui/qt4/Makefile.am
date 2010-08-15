INCLUDES=-I$(gwenhywfar_symlinkdir) $(qt4_includes)

AM_CPPFLAGS= @visibility_cflags@
DEFS=-DBUILDING_QT4_GUI

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwengui-qt4.pc


lib_LTLIBRARIES=libgwengui-qt4.la
noinst_PROGRAMS=libtest

noinst_HEADERS=\
  qt4dialogbox.hpp \
  qt4_gui_dialog.hpp \
  qt4_gui_dialog.hpp

libgwengui_qt4_la_LIBADD=$(qt4_libs) $(top_builddir)/src/$(gwenhywfar_internal_libname) $(builddir)/../cpp/libgwengui-cpp.la
libgwengui_qt4_la_SOURCES=\
  qt4dialogbox.cpp \
  qt4_gui.cpp \
  qt4_gui_dialog.cpp
  

nodist_libgwengui_qt4_la_SOURCES=

nodist_noinst_HEADERS=


iheaderdir=${gwenhywfar_headerdir_am}/gwen-gui-qt4
iheader_HEADERS=\
 qt4_gui.hpp

MOC_FILES=\
  qt4dialogbox.moc


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
libtest_LDADD=libgwengui-qt4.la $(top_builddir)/src/$(gwenhywfar_internal_libname) $(qt4_libs) \
  $(builddir)/../testdialogs/libgwengui-test.la



SUFFIXES = .ui .ui.hpp .ui.cpp .moc


%.moc: %.hpp
	$(qt4_moc) -o $@ $<

%.ui.hpp: %.ui
	$(qt4_uic) $< -o $@


sources:
	for f in $(libgwengui_qt4_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;


BUILT_SOURCES = $(MOC_FILES) $(nodist_libgwengui_qt4_la_SOURCES)
CLEANFILES = $(BUILT_SOURCES)


