# ===========================================================================
# tools/gsa/CMakeLists.txt
# ===========================================================================

add_executable(gsa
  add.c
  create.c
  check.c
  extract.c
  list.c
  mkkey.c
  rfi.c
  sign.c
  verify.c
  main.c
)

target_include_directories(gsa PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
)

target_link_libraries(gsa PRIVATE gwenhywfar)

if(WIN32 AND MINGW)
  target_link_options(gsa PRIVATE -mconsole)
endif()

install(TARGETS gsa DESTINATION ${CMAKE_INSTALL_BINDIR})
