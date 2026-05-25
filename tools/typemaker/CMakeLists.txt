# ===========================================================================
# tools/typemaker/CMakeLists.txt
# ===========================================================================

add_executable(typemaker
  args.c
  main.c
  phc.c
  lhc.c
  ahc.c
  code_c.c
)

target_include_directories(typemaker PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
)

target_link_libraries(typemaker PRIVATE gwenhywfar ${i18n_libs})

if(WIN32 AND MINGW)
  target_link_options(typemaker PRIVATE -mconsole)
endif()

install(TARGETS typemaker DESTINATION ${CMAKE_INSTALL_BINDIR})
