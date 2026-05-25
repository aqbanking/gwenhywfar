# src/crypt3 — cryptography sub-module (OBJECT library)
add_library(gwen_crypt3 OBJECT
  cryptalgo.c
  cryptdefs.c
  hashalgo.c
  paddalgo.c
  cryptkey.c
  cryptkeyrsa.c
  cryptkeysym.c
  mdigest.c
  mdigestgc.c
  padd.c
  smalltresor.c
)

target_include_directories(gwen_crypt3 PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/parser
  ${LIBGCRYPT_INCLUDE_DIRS}
  ${GPG_ERROR_INCLUDE_DIRS}
)

target_compile_definitions(gwen_crypt3 PRIVATE BUILDING_GWENHYWFAR)
