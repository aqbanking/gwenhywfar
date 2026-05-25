# src/cryptmsg — crypto message sub-module (OBJECT library)
add_library(gwen_cryptmsg OBJECT
  sighead.c
  sigtail.c
  crypthead.c
  cryptmgr.c
  cryptmgrkeys.c
)

target_include_directories(gwen_cryptmsg PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/parser
  ${LIBGCRYPT_INCLUDE_DIRS}
)

target_compile_definitions(gwen_cryptmsg PRIVATE BUILDING_GWENHYWFAR)
