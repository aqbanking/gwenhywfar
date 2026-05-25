# src/test_framework — test framework sub-module (OBJECT library)
add_library(gwen_test_framework OBJECT
  testmodule.c
  testframework.c
  testgui.c
)

target_include_directories(gwen_test_framework PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/parser
  ${LIBGNUTLS_INCLUDE_DIRS}
)

target_compile_definitions(gwen_test_framework PRIVATE BUILDING_GWENHYWFAR)
