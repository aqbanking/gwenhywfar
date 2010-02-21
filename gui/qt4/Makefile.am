INCLUDES=-I$(top_builddir)/gwenhywfar3 $(qt4_includes)

AM_CPPFLAGS= @visibility_cflags@
DEFS=-DBUILDING_QT4_GUI

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwengui-qt4.pc


lib_LTLIBRARIES=libgwengui-qt4.la
noinst_PROGRAMS=libtest

noinst_HEADERS=\
  cppdialog_p.hpp \
  cppgui_p.hpp \
  qt4dialogbox.hpp \
  qt4_gui_dialog.hpp \
  qt4_gui_dialog.hpp

libgwengui_qt4_la_LIBADD=$(qt4_libs) $(top_builddir)/src/$(gwenhywfar_internal_libname)
libgwengui_qt4_la_SOURCES=\
  cppdialog.cpp \
  cppgui.cpp \
  qt4dialogbox.cpp \
  qt4_gui.cpp \
  qt4_gui_dialog.cpp
  

nodist_libgwengui_qt4_la_SOURCES=

nodist_noinst_HEADERS=


iheaderdir=${includedir}/gwen-gui-qt4
iheader_HEADERS=\
 cppgui.hpp \
 cppdialog.hpp \
 qt4_gui.hpp

MOC_FILES=\
 qt4_gui_dialog.moc

UI_FILES=

EXTRA_DIST=$(UI_FILES) testdialog.xml


libtest_SOURCES=libtest.cpp
libtest_LDADD=libgwengui-qt4.la $(top_builddir)/src/$(gwenhywfar_internal_libname) $(qt_libs) -lQtCore



SUFFIXES = .ui .ui.hpp .ui.cpp .moc


%.moc: %.hpp
	$(qt4_moc) -o $@ $<

%.ui.cpp: %.ui
	$(UIC3) $< -impl $(@:.ui.cpp=.ui.hpp) -o $@.tmp && \
	echo "#ifdef HAVE_CONFIG_H" >$@ && \
	echo "# include <config.h>" >>$@ && \
	echo "#endif" >>$@ && \
	cat $@.tmp >>$@ && \
	rm $@.tmp && \
	echo "#include \"`basename $<`.moc\"" >>$@


%.ui.hpp: %.ui
	$(UIC3) $< -o $@


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


