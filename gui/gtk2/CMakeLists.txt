# ===========================================================================
# gui/gtk2/CMakeLists.txt — libgwengui-gtk2 shared library
# ===========================================================================

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK2 REQUIRED "gtk+-2.0 >= 2.17.5")

# ---------------------------------------------------------------------------
# Shared library
# ---------------------------------------------------------------------------
add_library(gwengui-gtk2 SHARED
  gtk2_gui_dialog.c
  gtk2_gui.c
)

target_include_directories(gwengui-gtk2 PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${GTK2_INCLUDE_DIRS}
)

target_compile_definitions(gwengui-gtk2 PRIVATE
  BUILDING_GTK2_GUI
  PACKAGE="gwenhywfar"
)

if(GCC_WITH_VISIBILITY_ATTRIBUTE)
  target_compile_options(gwengui-gtk2 PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(gwengui-gtk2 PUBLIC
  ${GTK2_LIBRARIES}
  gwenhywfar
)

set_target_properties(gwengui-gtk2 PROPERTIES
  VERSION   "${GWENHYWFAR_SO_EFFECTIVE}.${GWENHYWFAR_SO_AGE}.${GWENHYWFAR_SO_REVISION}"
  SOVERSION "${GWENHYWFAR_SO_EFFECTIVE}"
  OUTPUT_NAME gwengui-gtk2
)

# ---------------------------------------------------------------------------
# Install library
# ---------------------------------------------------------------------------
install(TARGETS gwengui-gtk2
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ---------------------------------------------------------------------------
# Install public headers
# ---------------------------------------------------------------------------
install(FILES
  gtk2_gui.h
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/gwen-gui-gtk2"
)

# ---------------------------------------------------------------------------
# Generate and install .pc file
# ---------------------------------------------------------------------------
gwen_generate_pc_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-gtk2.pc.in"
  "gwengui-gtk2.pc"
)

# ---------------------------------------------------------------------------
# libtest — noinst test program (NOT installed)
# ---------------------------------------------------------------------------
add_executable(gtk2_libtest libtest.c)
target_include_directories(gtk2_libtest PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${GTK2_INCLUDE_DIRS}
)
target_link_libraries(gtk2_libtest PRIVATE
  gwengui-gtk2
  gwengui-test
  gwenhywfar
  ${GTK2_LIBRARIES}
)
