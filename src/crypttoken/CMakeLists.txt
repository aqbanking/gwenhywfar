# src/crypttoken — crypto token sub-module (OBJECT library)
add_library(gwen_crypttoken OBJECT
  ct_context.c
  ct_keyinfo.c
  ct.c
  ctf_context.c
  ctfile.c
  ctplugin.c
)

target_include_directories(gwen_crypttoken PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/crypt3
  ${CMAKE_SOURCE_DIR}/src/parser
)

target_compile_definitions(gwen_crypttoken PRIVATE
  BUILDING_GWENHYWFAR
  LIBCHIPCARD_GWENHYWFAR_PLUGIN_PACKAGE="libchipcard-libgwenhywfar${GWENHYWFAR_SO_EFFECTIVE}-plugins"
)
