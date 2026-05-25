# ===========================================================================
# tools/typemaker2/CMakeLists.txt
# ===========================================================================

add_subdirectory(c)

add_executable(typemaker2
  tm_type.c
  tm_member.c
  tm_header.c
  tm_util.c
  tm_typemanager.c
  tm_builder.c
  tm_item.c
  tm_enum.c
  tm_define.c
  tm_code.c
  tm_inline.c
  tm_virtualfn.c
  tm_signal.c
  tm_slot.c
  tm_group.c
  build.c
  builddefs.c
  main.c
)

target_include_directories(typemaker2 PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/c
)

target_link_libraries(typemaker2 PRIVATE tm2_c gwenhywfar)

if(WIN32 AND MINGW)
  target_link_options(typemaker2 PRIVATE -mconsole)
endif()

install(TARGETS typemaker2 DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install .tm2 type definition data files
file(GLOB _tm2_data_files "${CMAKE_CURRENT_SOURCE_DIR}/types/c/*.tm2")
install(FILES ${_tm2_data_files}
  DESTINATION ${CMAKE_INSTALL_DATADIR}/gwenhywfar/typemaker2/c
)
