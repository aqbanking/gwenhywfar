# ===========================================================================
# tools/gwenbuild/buildctx/CMakeLists.txt
# ===========================================================================

add_library(gwb_build STATIC
  buildcmd.c
  buildsubcmd.c
  buildctx.c
  buildctx_bdeps.c
  buildctx_depfile.c
  buildctx_run.c
  buildctx_xml.c
)

target_include_directories(gwb_build PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tools
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)
