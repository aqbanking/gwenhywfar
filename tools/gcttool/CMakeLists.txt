# ===========================================================================
# tools/gcttool/CMakeLists.txt
# ===========================================================================

add_executable(gct-tool
  activatekey.c
  changepin.c
  create.c
  genkey.c
  showkey.c
  showuser.c
  update.c
  setsignseq.c
  setkey.c
  hashtree.c
  checktree.c
  showpasswords.c
  main.c
)

target_include_directories(gct-tool PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
)

target_link_libraries(gct-tool PRIVATE gwenhywfar)

if(HAVE_OPENSSL)
  target_include_directories(gct-tool PRIVATE ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(gct-tool PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if(WIN32 AND MINGW)
  target_link_options(gct-tool PRIVATE -mconsole)
endif()

install(TARGETS gct-tool DESTINATION ${CMAKE_INSTALL_BINDIR})

# cttest — noinst test program (not installed)
add_executable(cttest cttest.c)
target_include_directories(cttest PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
)
target_link_libraries(cttest PRIVATE gwenhywfar ${i18n_libs})
