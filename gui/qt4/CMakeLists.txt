# ===========================================================================
# gui/qt4/CMakeLists.txt — libgwengui-qt4 shared library
# ===========================================================================

find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui)

# MOC generation
set(CMAKE_AUTOMOC OFF)
qt4_wrap_cpp(_moc_sources qt4dialogbox.hpp)

# ---------------------------------------------------------------------------
# Shared library
# ---------------------------------------------------------------------------
add_library(gwengui-qt4 SHARED
  qt4dialogbox.cpp
  qt4_gui.cpp
  qt4_gui_dialog.cpp
  ${_moc_sources}
)

target_include_directories(gwengui-qt4 PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QT_INCLUDES}
)

target_compile_definitions(gwengui-qt4 PRIVATE
  BUILDING_QT4_GUI
)

if(GCC_WITH_VISIBILITY_ATTRIBUTE)
  target_compile_options(gwengui-qt4 PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(gwengui-qt4 PUBLIC
  Qt4::QtCore
  Qt4::QtGui
  gwenhywfar
  gwengui-cpp
)

set_target_properties(gwengui-qt4 PROPERTIES
  VERSION   "${GWENHYWFAR_SO_EFFECTIVE}.${GWENHYWFAR_SO_AGE}.${GWENHYWFAR_SO_REVISION}"
  SOVERSION "${GWENHYWFAR_SO_EFFECTIVE}"
  OUTPUT_NAME gwengui-qt4
)

# ---------------------------------------------------------------------------
# Install library
# ---------------------------------------------------------------------------
install(TARGETS gwengui-qt4
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ---------------------------------------------------------------------------
# Install public headers
# ---------------------------------------------------------------------------
install(FILES
  qt4_gui.hpp
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/gwen-gui-qt4"
)

# ---------------------------------------------------------------------------
# Generate and install .pc file
# ---------------------------------------------------------------------------
gwen_generate_pc_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-qt4.pc.in"
  "gwengui-qt4.pc"
)

# ---------------------------------------------------------------------------
# Generate and install cmake config files via template conversion
# ---------------------------------------------------------------------------
gwen_convert_autotools_template(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-qt4-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt4-config.cmake.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt4-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt4-config.cmake"
  @ONLY
)

gwen_convert_autotools_template(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-qt4-config-version.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt4-config-version.cmake.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt4-config-version.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt4-config-version.cmake"
  @ONLY
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt4-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt4-config-version.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gwengui-qt4-${GWENHYWFAR_VERSION_MAJOR}.${GWENHYWFAR_VERSION_MINOR}"
)

# ---------------------------------------------------------------------------
# libtest — noinst test program (NOT installed)
# ---------------------------------------------------------------------------
add_executable(qt4_libtest libtest.cpp)
target_include_directories(qt4_libtest PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QT_INCLUDES}
)
target_link_libraries(qt4_libtest PRIVATE
  gwengui-qt4
  gwengui-cpp
  gwengui-test
  gwenhywfar
  Qt4::QtCore
  Qt4::QtGui
)
