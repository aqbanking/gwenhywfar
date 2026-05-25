# ===========================================================================
# tools/gwenbuild/parser/CMakeLists.txt
# ===========================================================================

add_library(gwb_parser STATIC
  parser.c
  p_buildfiles.c
  p_project.c
  p_checkfunctions.c
  p_checkheaders.c
  p_checklibs.c
  p_checkprogs.c
  p_checkcompiler.c
  p_dependencies.c
  p_options.c
  p_target.c
)

target_include_directories(gwb_parser PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tools
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)
