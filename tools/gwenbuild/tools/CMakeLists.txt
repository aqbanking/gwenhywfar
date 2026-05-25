# ===========================================================================
# tools/gwenbuild/tools/CMakeLists.txt
# ===========================================================================

add_library(gwb_tools STATIC
  trycompile.c
  trylink.c
)

target_include_directories(gwb_tools PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tools
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)
