INCLUDES=-I$(gwenhywfar_symlinkdir) $(COCOA_CFLAGS)

AM_CPPFLAGS= @visibility_cflags@
DEFS+=-DBUILDING_COCOA_GUI -DPACKAGE=\"$(PACKAGE)\"

AM_OBJCFLAGS= -x objective-c

lib_LTLIBRARIES=libgwengui-cocoa.la


libgwengui_cocoa_la_LIBADD=$(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(COCOA_FRAMEWORK) $(FOUNDATION_FRAMEWORK) $(APPKIT_FRAMEWORK) $(COREDATA_FRAMEWORK) $(WEBKIT_FRAMEWORK) \
  -lobjc

#libgwengui_cocoa_la_LDFLAGS=
noinst_HEADERS=\
  CocoaButton.h \
  CocoaCheckboxButton.h \
  CocoaComboBox.h \
  CocoaGridLayout.h \
  CocoaGroupBox.h \
  cocoa_gui_dialog_l.h \
  cocoa_gui_dialog_p.h \
  cocoa_gui.h \
  cocoa_gui_p.h \
  CocoaGwenGUIProtocol.h \
  CocoaHLayout.h \
  CocoaHLineView.h \
  CocoaLabel.h \
  CocoaLineTextField.h \
  CocoaListBox.h \
  CocoaPopUpButton.h \
  CocoaProgressbar.h \
  CocoaRadioButton.h \
  CocoaScrollBox.h \
  CocoaSpacer.h \
  CocoaSpinbox.h \
  CocoaTabView.h \
  CocoaTextBrowser.h \
  CocoaVLayout.h \
  CocoaVLineView.h \
  CocoaWindowContentView.h \
  CocoaWindow.h


libgwengui_cocoa_la_SOURCES=\
  CocoaButton.m \
  CocoaCheckboxButton.m \
  CocoaComboBox.m \
  CocoaGridLayout.m \
  CocoaGroupBox.m \
  cocoa_gui_dialog.m \
  cocoa_gui.m \
  CocoaHLayout.m \
  CocoaHLineView.m \
  CocoaLabel.m \
  CocoaLineTextField.m \
  CocoaListBox.m \
  CocoaPopUpButton.m \
  CocoaProgressbar.m \
  CocoaRadioButton.m \
  CocoaScrollBox.m \
  CocoaSpacer.m \
  CocoaSpinbox.m \
  CocoaTabView.m \
  CocoaTextBrowser.m \
  CocoaVLayout.m \
  CocoaVLineView.m \
  CocoaWindowContentView.m \
  CocoaWindow.m


iheaderdir=${gwenhywfar_headerdir_am}/gwen-gui-cocoa
iheader_HEADERS= \
  cocoa_gui.h


noinst_PROGRAMS = libtest
libtest_SOURCES = libtest.m
#libtest_LDFLAGS=
libtest_LDADD = libgwengui-cocoa.la $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(COCOA_FRAMEWORK) $(FOUNDATION_FRAMEWORK) $(APPKIT_FRAMEWORK) $(COREDATA_FRAMEWORK) $(WEBKIT_FRAMEWORK) \
  -lobjc \
  $(builddir)/../testdialogs/libgwengui-test.la


EXTRA_DIST=\
  w_dialog.mm \
  w_label.mm \
  w_vlayout.mm \
  w_hlayout.mm \
  w_pushbutton.mm \
  w_lineedit.mm \
  w_textedit.mm \
  w_checkbox.mm \
  w_hline.mm \
  w_vline.mm \
  w_progressbar.mm \
  w_radiobutton.mm \
  w_tabbook.mm \
  w_groupbox.mm \
  w_combobox.mm \
  w_gridlayout.mm \
  w_hspacer.mm \
  w_vspacer.mm \
  w_listbox.mm \
  w_scrollarea.mm \
  w_textbrowser.mm \
  w_stack.mm \
  w_spinbox.mm


