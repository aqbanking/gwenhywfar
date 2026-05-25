# ===========================================================================
# tools/gwenbuild/CMakeLists.txt — POSIX only
# ===========================================================================

# Sub-libraries
add_subdirectory(types)
add_subdirectory(parser)
add_subdirectory(tools)
add_subdirectory(buildctx)
add_subdirectory(builders)

add_executable(gwbuild
  main.c
  utils.c
  c_setup.c
  c_prepare.c
  c_build.c
  c_install.c
  c_clean.c
  c_dist.c
)

target_include_directories(gwbuild PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tools
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gwbuild PRIVATE
  gwenhywfar
  gwb_types
  gwb_parser
  gwb_tools
  gwb_builders
  gwb_build
)

install(TARGETS gwbuild DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install template files
install(FILES
  templates/project.tmpl
  templates/convlib.tmpl
  templates/module.tmpl
  DESTINATION ${CMAKE_INSTALL_DATADIR}/gwenbuild/templates
)

# Install builder definition files
file(GLOB _posix_gwb "${CMAKE_CURRENT_SOURCE_DIR}/builders/posix/*.gwb")
file(GLOB _windows_gwb "${CMAKE_CURRENT_SOURCE_DIR}/builders/windows/*.gwb")

install(FILES ${_posix_gwb}
  DESTINATION ${CMAKE_INSTALL_DATADIR}/gwenhywfar/gwenbuild/builders/posix
)
install(FILES ${_windows_gwb}
  DESTINATION ${CMAKE_INSTALL_DATADIR}/gwenhywfar/gwenbuild/builders/windows
)
