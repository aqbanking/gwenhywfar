# ===========================================================================
# tools/gwenbuild/types/CMakeLists.txt
# ===========================================================================

add_library(gwb_types STATIC
  builder.c
  context.c
  file.c
  gwenbuild.c
  keyvaluepair.c
  target.c
  project.c
  option.c
)

target_include_directories(gwb_types PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tools
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(gwb_types PRIVATE
  BUILDERDATADIR="${CMAKE_INSTALL_FULL_DATADIR}/gwenhywfar/gwenbuild/builders"
)
