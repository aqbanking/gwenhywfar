# ===========================================================================
# gui/qt5/CMakeLists.txt — libgwengui-qt5 or libgwengui-qt6 shared library
# ===========================================================================

# Determine Qt major version from the GWEN_GUI_BACKENDS list
if("qt6" IN_LIST GWEN_GUI_BACKENDS)
  set(QT_MAJOR_VERSION 6)
  set(QT_VER 6)
else()
  set(QT_MAJOR_VERSION 5)
  set(QT_VER 5)
endif()

find_package(Qt${QT_VER} REQUIRED COMPONENTS Widgets Core)

# MOC generation: qt5dialogbox.cpp uses the old-style #include "qt5dialogbox.moc"
# pattern, so we need to generate qt5dialogbox.moc from the header and place it
# where the .cpp can find it via include path.
set(CMAKE_AUTOMOC OFF)
if(QT_VER EQUAL 6)
  set(_qt_moc_exe Qt6::moc)
else()
  set(_qt_moc_exe Qt5::moc)
endif()

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt5dialogbox.moc"
  COMMAND ${_qt_moc_exe}
    ${CMAKE_CURRENT_SOURCE_DIR}/qt5dialogbox.hpp
    -o ${CMAKE_CURRENT_BINARY_DIR}/qt5dialogbox.moc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/qt5dialogbox.hpp
  COMMENT "Running MOC on qt5dialogbox.hpp"
)

# ---------------------------------------------------------------------------
# Shared library
# ---------------------------------------------------------------------------
add_library(gwengui-qt${QT_VER} SHARED
  qt5dialogbox.cpp
  qt5_gui.cpp
  qt5_gui_dialog.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/qt5dialogbox.moc
)

target_include_directories(gwengui-qt${QT_VER} PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gwengui-qt${QT_VER} PRIVATE
  BUILDING_QT${QT_VER}_GUI
)
# Qt6 shares the Qt5 source which checks BUILDING_QT5_GUI for symbol export
if(QT_VER EQUAL 6)
  target_compile_definitions(gwengui-qt${QT_VER} PRIVATE BUILDING_QT5_GUI)
endif()

if(GCC_WITH_VISIBILITY_ATTRIBUTE)
  target_compile_options(gwengui-qt${QT_VER} PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(gwengui-qt${QT_VER} PUBLIC
  Qt${QT_VER}::Widgets
  Qt${QT_VER}::Core
  gwenhywfar
  gwengui-cpp
)

set_target_properties(gwengui-qt${QT_VER} PROPERTIES
  VERSION   "${GWENHYWFAR_SO_EFFECTIVE}.${GWENHYWFAR_SO_AGE}.${GWENHYWFAR_SO_REVISION}"
  SOVERSION "${GWENHYWFAR_SO_EFFECTIVE}"
  OUTPUT_NAME gwengui-qt${QT_VER}
)

# ---------------------------------------------------------------------------
# Install library
# ---------------------------------------------------------------------------
install(TARGETS gwengui-qt${QT_VER}
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ---------------------------------------------------------------------------
# Install public headers
# ---------------------------------------------------------------------------
install(FILES
  qt5_gui.hpp
  qt5_gui_dialog.hpp
  qt5dialogbox.hpp
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/gwen-gui-qt5"
)

# ---------------------------------------------------------------------------
# Generate and install .pc file (qt5 template serves both qt5 and qt6)
# ---------------------------------------------------------------------------
gwen_generate_pc_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-qt5.pc.in"
  "gwengui-qt${QT_VER}.pc"
)

# ---------------------------------------------------------------------------
# Generate and install cmake config files via template conversion
# The qt5 templates use @QT_MAJOR_VERSION@ and serve both qt5 and qt6 builds.
# The qt6 .in files in the source tree are just references to the qt5 ones.
# ---------------------------------------------------------------------------
gwen_convert_autotools_template(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-qt5-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt${QT_VER}-config.cmake.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt${QT_VER}-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt${QT_VER}-config.cmake"
  @ONLY
)

gwen_convert_autotools_template(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-qt5-config-version.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt${QT_VER}-config-version.cmake.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt${QT_VER}-config-version.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt${QT_VER}-config-version.cmake"
  @ONLY
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt${QT_VER}-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-qt${QT_VER}-config-version.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gwengui-qt${QT_VER}-${GWENHYWFAR_VERSION_MAJOR}.${GWENHYWFAR_VERSION_MINOR}"
)

# ---------------------------------------------------------------------------
# libtest — noinst test program (NOT installed)
# ---------------------------------------------------------------------------
add_executable(qt${QT_VER}_libtest libtest.cpp)
target_include_directories(qt${QT_VER}_libtest PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(qt${QT_VER}_libtest PRIVATE
  gwengui-qt${QT_VER}
  gwengui-cpp
  gwengui-test
  gwenhywfar
  Qt${QT_VER}::Widgets
  Qt${QT_VER}::Core
)
