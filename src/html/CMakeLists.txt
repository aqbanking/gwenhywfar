# src/html — HTML rendering sub-module (OBJECT library)
add_library(gwen_html OBJECT
  g_box.c
  g_generic.c
  g_table.c
  g_tablerow.c
  g_unorderedlist.c
  htmlctx.c
  htmlfont.c
  htmlimage.c
  htmlgroup.c
  htmlobject.c
  htmlprops.c
  o_box.c
  o_grid.c
  o_gridentry.c
  o_image.c
  o_word.c
)

target_include_directories(gwen_html PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(gwen_html PRIVATE BUILDING_GWENHYWFAR)
