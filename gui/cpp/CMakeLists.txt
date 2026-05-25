# ===========================================================================
# gui/cpp/CMakeLists.txt — libgwengui-cpp shared library
# ===========================================================================

add_library(gwengui-cpp SHARED
  cppdialog.cpp
  cppgui.cpp
  cppwidget.cpp
)

target_include_directories(gwengui-cpp PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(gwengui-cpp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gwenhywfar5>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/gwen-gui-cpp>
)

target_compile_definitions(gwengui-cpp PRIVATE
  BUILDING_CPP_GUI
  PACKAGE="gwenhywfar"
)

if(GCC_WITH_VISIBILITY_ATTRIBUTE)
  target_compile_options(gwengui-cpp PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(gwengui-cpp PUBLIC gwenhywfar)

set_target_properties(gwengui-cpp PROPERTIES
  VERSION   "${GWENHYWFAR_SO_EFFECTIVE}.${GWENHYWFAR_SO_AGE}.${GWENHYWFAR_SO_REVISION}"
  SOVERSION "${GWENHYWFAR_SO_EFFECTIVE}"
  OUTPUT_NAME gwengui-cpp
)

# ---------------------------------------------------------------------------
# Install library
# ---------------------------------------------------------------------------
install(TARGETS gwengui-cpp
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ---------------------------------------------------------------------------
# Install public headers
# ---------------------------------------------------------------------------
install(FILES
  api.h
  cppgui.hpp
  cppdialog.hpp
  cppwidget.hpp
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/gwen-gui-cpp"
)

# ---------------------------------------------------------------------------
# Generate and install cmake config files via template conversion
# ---------------------------------------------------------------------------
gwen_convert_autotools_template(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-cpp-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-cpp-config.cmake.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-cpp-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-cpp-config.cmake"
  @ONLY
)

gwen_convert_autotools_template(
  "${CMAKE_CURRENT_SOURCE_DIR}/gwengui-cpp-config-version.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-cpp-config-version.cmake.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-cpp-config-version.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-cpp-config-version.cmake"
  @ONLY
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-cpp-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/gwengui-cpp-config-version.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gwengui-cpp-${GWENHYWFAR_VERSION_MAJOR}.${GWENHYWFAR_VERSION_MINOR}"
)
