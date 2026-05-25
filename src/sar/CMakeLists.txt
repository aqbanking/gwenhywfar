# src/sar — simple archive sub-module (OBJECT library)
add_library(gwen_sar OBJECT
  gwen_sar_fileheader.c
  sar.c
)

target_include_directories(gwen_sar PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/sio
  ${CMAKE_SOURCE_DIR}/src/parser
)

target_compile_definitions(gwen_sar PRIVATE BUILDING_GWENHYWFAR)
