INCLUDES=-I$(gwenhywfar_symlinkdir) $(qt3_includes)

AM_CPPFLAGS= @visibility_cflags@
DEFS=-DBUILDING_QT3_GUI

lib_LTLIBRARIES=libgwengui-qt3.la
noinst_PROGRAMS=libtest

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwengui-qt3.pc


UI_FILES=\
  qt3_printdialog.ui

noinst_HEADERS=\
  cppdialog_p.hpp \
  cppgui_p.hpp \
  qt3_printdialog.hpp \
  qt3dialogbox.hpp \
  qt3_gui_dialog.hpp \
  qt3_gui_dialog.hpp

libgwengui_qt3_la_LIBADD=$(qt3_libs) $(top_builddir)/src/$(gwenhywfar_internal_libname)
libgwengui_qt3_la_SOURCES=\
  cppdialog.cpp \
  cppgui.cpp \
  qt3_printdialog.cpp \
  qt3dialogbox.cpp \
  qt3_gui.cpp \
  qt3_gui_dialog.cpp
  

nodist_libgwengui_qt3_la_SOURCES=\
  qt3_printdialog.ui.cpp

nodist_noinst_HEADERS=


iheaderdir=${gwenhywfar_headerdir_am}/gwen-gui-qt3
iheader_HEADERS=\
 cppgui.hpp \
 cppdialog.hpp \
 qt3_gui.hpp

nodist_iheader_HEADERS=\
  qt3_printdialog.ui.hpp


MOC_FILES=\
 qt3_gui_dialog.moc \
 qt3_printdialog.moc \
 qt3_printdialog.ui.moc


EXTRA_DIST=$(UI_FILES)


libtest_SOURCES=libtest.cpp
libtest_LDADD=libgwengui-qt3.la $(top_builddir)/src/$(gwenhywfar_internal_libname) $(qt3_libs) \
  $(builddir)/../testdialogs/libgwengui-test.la




SUFFIXES = .ui .ui.hpp .ui.cpp .moc


%.moc: %.hpp
	$(qt3_moc) -o $@ $<

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
	for f in $(libgwengui_qt3_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;


BUILT_SOURCES = $(MOC_FILES) $(nodist_libgwengui_qt3_la_SOURCES)
CLEANFILES = $(BUILT_SOURCES)


