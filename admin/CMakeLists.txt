# ===========================================================================
# admin/CMakeLists.txt — mklistdoc tool
# ===========================================================================

add_executable(mklistdoc mklistdoc.c)

target_include_directories(mklistdoc PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
)

target_compile_definitions(mklistdoc PRIVATE
  GWEN_HEADERS="${gwenhywfar_headerdir}/gwenhywfar"
)

if(ENABLE_ERROR_ON_WARNING)
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(mklistdoc PRIVATE -Werror)
  elseif(MSVC)
    target_compile_options(mklistdoc PRIVATE /WX)
  endif()
endif()

install(TARGETS mklistdoc DESTINATION ${CMAKE_INSTALL_BINDIR})
