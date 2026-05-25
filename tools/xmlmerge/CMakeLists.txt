# ===========================================================================
# tools/xmlmerge/CMakeLists.txt
# ===========================================================================

add_executable(xmlmerge args.c main.c)

target_include_directories(xmlmerge PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
)

target_link_libraries(xmlmerge PRIVATE gwenhywfar ${i18n_libs})

if(WIN32 AND MINGW)
  target_link_options(xmlmerge PRIVATE -mconsole)
endif()

install(TARGETS xmlmerge DESTINATION ${CMAKE_INSTALL_BINDIR})
