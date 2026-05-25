# ===========================================================================
# tools/gwenbuild/builders/CMakeLists.txt
# ===========================================================================

add_library(gwb_builders STATIC
  genericbuilder.c
  gbuilderdescr.c
)

target_include_directories(gwb_builders PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tools
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(gwb_builders PRIVATE
  BUILDERDATADIR="${CMAKE_INSTALL_FULL_DATADIR}/gwenhywfar/gwenbuild/builders"
)
