# ===========================================================================
# gui/fox16/CMakeLists.txt — libgwengui-fox16 shared library
# ===========================================================================

find_package(PkgConfig REQUIRED)
pkg_check_modules(FOX REQUIRED "fox >= 1.6")

# ---------------------------------------------------------------------------
# Shared library
# ---------------------------------------------------------------------------
add_library(gwengui-fox16 SHARED
  fox16_gui.cpp
  fox16_gui_dialog.cpp
  fox16_gui_sortinglist.cpp
  fox16_gui_updater.cpp
  fox16_htmlctx.cpp
  fox16_htmllabel.cpp
  fox16_htmltext.cpp
  theme.cpp
)

target_include_directories(gwengui-fox16 PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${FOX_INCLUDE_DIRS}
)

target_compile_definitions(gwengui-fox16 PRIVATE
  BUILDING_FOX16_GUI
  PACKAGE="gwenhywfar"
)

if(GCC_WITH_VISIBILITY_ATTRIBUTE)
  target_compile_options(gwengui-fox16 PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(gwengui-fox16 PUBLIC
  ${FOX_LIBRARIES}
  gwenhywfar
  gwengui-cpp
)

set_target_properties(gwengui-fox16 PROPERTIES
  VERSION   "${GWENHYWFAR_SO_EFFECTIVE}.${GWENHYWFAR_SO_AGE}.${GWENHYWFAR_SO_REVISION}"
  SOVERSION "${GWENHYWFAR_SO_EFFECTIVE}"
  OUTPUT_NAME gwengui-fox16
)

# ---------------------------------------------------------------------------
# Install library
# ---------------------------------------------------------------------------
install(TARGETS gwengui-fox16
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ---------------------------------------------------------------------------
# Install public headers
# ---------------------------------------------------------------------------
install(FILES
  fox16_gui.hpp
  fox16_htmlctx.hpp
  fox16_htmllabel.hpp
  fox16_htmltext.hpp
  theme.h
  api.h
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/gwen-gui-fox16"
)

# ---------------------------------------------------------------------------
# Generate and install .pc file
# ---------------------------------------------------------------------------
gwen_generate_pc_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-fox16.pc.in"
  "gwengui-fox16.pc"
)

# ---------------------------------------------------------------------------
# libtest — noinst test program (NOT installed)
# ---------------------------------------------------------------------------
add_executable(fox16_libtest libtest.cpp)
target_include_directories(fox16_libtest PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${FOX_INCLUDE_DIRS}
)
target_link_libraries(fox16_libtest PRIVATE
  gwengui-fox16
  gwengui-cpp
  gwengui-test
  gwenhywfar
  ${FOX_LIBRARIES}
)
