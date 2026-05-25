# src/parser — parser sub-module (OBJECT library)
add_library(gwen_parser OBJECT
  base64.c
  configmgr.c
  db.c
  dbio.c
  msgengine.c
  path.c
  tag16.c
  text.c
  tlv.c
  url.c
  urlfns.c
  xml.c
  xmlctx.c
  xsdnode.c
  xml2db.c
  json.c
  json_read.c
  json_dump.c
)

target_include_directories(gwen_parser PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/os
  ${CMAKE_SOURCE_DIR}/src/parser
)

target_compile_definitions(gwen_parser PRIVATE BUILDING_GWENHYWFAR)
