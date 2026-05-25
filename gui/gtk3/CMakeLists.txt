# ===========================================================================
# gui/gtk3/CMakeLists.txt — libgwengui-gtk3 shared library
# ===========================================================================

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED "gtk+-3.0 >= 3.10.8")

# ---------------------------------------------------------------------------
# Shared library
# ---------------------------------------------------------------------------
add_library(gwengui-gtk3 SHARED
  gtk3_gui_dialog.c
  gtk3_gui.c
)

target_include_directories(gwengui-gtk3 PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${GTK3_INCLUDE_DIRS}
)

target_compile_definitions(gwengui-gtk3 PRIVATE
  BUILDING_GTK3_GUI
  PACKAGE="gwenhywfar"
)

if(GCC_WITH_VISIBILITY_ATTRIBUTE)
  target_compile_options(gwengui-gtk3 PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(gwengui-gtk3 PUBLIC
  ${GTK3_LIBRARIES}
  gwenhywfar
)

set_target_properties(gwengui-gtk3 PROPERTIES
  VERSION   "${GWENHYWFAR_SO_EFFECTIVE}.${GWENHYWFAR_SO_AGE}.${GWENHYWFAR_SO_REVISION}"
  SOVERSION "${GWENHYWFAR_SO_EFFECTIVE}"
  OUTPUT_NAME gwengui-gtk3
)

# ---------------------------------------------------------------------------
# Install library
# ---------------------------------------------------------------------------
install(TARGETS gwengui-gtk3
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ---------------------------------------------------------------------------
# Install public headers
# ---------------------------------------------------------------------------
install(FILES
  gtk3_gui.h
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/gwen-gui-gtk3"
)

# ---------------------------------------------------------------------------
# Generate and install .pc file
# ---------------------------------------------------------------------------
gwen_generate_pc_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-gtk3.pc.in"
  "gwengui-gtk3.pc"
)

# ---------------------------------------------------------------------------
# libtest — noinst test program (NOT installed)
# ---------------------------------------------------------------------------
add_executable(gtk3_libtest libtest.c)
target_include_directories(gtk3_libtest PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${GTK3_INCLUDE_DIRS}
)
target_link_libraries(gtk3_libtest PRIVATE
  gwengui-gtk3
  gwengui-test
  gwenhywfar
  ${GTK3_LIBRARIES}
)
