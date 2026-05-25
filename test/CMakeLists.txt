# ===========================================================================
# test/CMakeLists.txt — gwentest (noinst test program, NOT installed)
# ===========================================================================

add_executable(gwentest
  gwentest.c
  testthread.c
)

target_include_directories(gwentest PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${LIBGCRYPT_INCLUDE_DIRS}
  ${GPG_ERROR_INCLUDE_DIRS}
  ${LIBGNUTLS_INCLUDE_DIRS}
)

target_link_libraries(gwentest PRIVATE
  gwenhywfar
  ${LIBGCRYPT_LIBRARIES}
  ${GPG_ERROR_LIBRARIES}
  ${LIBGNUTLS_LIBRARIES}
)
