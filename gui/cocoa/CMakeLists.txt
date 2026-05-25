# ===========================================================================
# gui/cocoa/CMakeLists.txt — libgwengui-cocoa shared library (macOS only)
# ===========================================================================

# ---------------------------------------------------------------------------
# Shared library (Objective-C sources)
# ---------------------------------------------------------------------------
add_library(gwengui-cocoa SHARED
  CocoaButton.m
  CocoaCheckboxButton.m
  CocoaComboBox.m
  CocoaGridLayout.m
  CocoaGroupBox.m
  cocoa_gui_dialog.m
  cocoa_gui.m
  CocoaHLayout.m
  CocoaHLineView.m
  CocoaLabel.m
  CocoaLineTextField.m
  CocoaListBox.m
  CocoaPopUpButton.m
  CocoaProgressbar.m
  CocoaRadioButton.m
  CocoaScrollBox.m
  CocoaSpacer.m
  CocoaSpinbox.m
  CocoaTabView.m
  CocoaTextBrowser.m
  CocoaVLayout.m
  CocoaVLineView.m
  CocoaWindowContentView.m
  CocoaWindow.m
)

target_include_directories(gwengui-cocoa PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(gwengui-cocoa PRIVATE
  BUILDING_COCOA_GUI
  PACKAGE="gwenhywfar"
)

# Link Apple frameworks and Objective-C runtime
target_link_libraries(gwengui-cocoa PUBLIC
  gwenhywfar
  "-framework Cocoa"
  "-framework CoreFoundation"
  "-framework AppKit"
  "-framework CoreData"
  "-framework WebKit"
  "-lobjc"
)

set_target_properties(gwengui-cocoa PROPERTIES
  VERSION   "${GWENHYWFAR_SO_EFFECTIVE}.${GWENHYWFAR_SO_AGE}.${GWENHYWFAR_SO_REVISION}"
  SOVERSION "${GWENHYWFAR_SO_EFFECTIVE}"
  OUTPUT_NAME gwengui-cocoa
)

# ---------------------------------------------------------------------------
# Install library
# ---------------------------------------------------------------------------
install(TARGETS gwengui-cocoa
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ---------------------------------------------------------------------------
# Install public headers
# ---------------------------------------------------------------------------
install(FILES
  cocoa_gui.h
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/gwen-gui-cocoa"
)

# ---------------------------------------------------------------------------
# gwen-cocoa-test — noinst test program (NOT installed)
# ---------------------------------------------------------------------------
add_executable(gwen-cocoa-test libtest.m)
target_include_directories(gwen-cocoa-test PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(gwen-cocoa-test PRIVATE
  gwengui-cocoa
  gwengui-test
  gwenhywfar
  "-framework Cocoa"
  "-framework CoreFoundation"
  "-framework AppKit"
  "-framework CoreData"
  "-framework WebKit"
  "-lobjc"
)
