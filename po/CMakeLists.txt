# ===========================================================================
# po/CMakeLists.txt — Internationalization (i18n)
# ===========================================================================

find_program(XGETTEXT_EXECUTABLE xgettext)
find_program(MSGFMT_EXECUTABLE msgfmt)

if(NOT XGETTEXT_EXECUTABLE)
  message(WARNING "xgettext not found — i18n file generation disabled")
endif()

if(NOT MSGFMT_EXECUTABLE)
  message(WARNING "msgfmt not found — i18n file generation disabled")
endif()

if(XGETTEXT_EXECUTABLE AND MSGFMT_EXECUTABLE)
  # Collect all .po files in this directory
  file(GLOB _po_files "${CMAKE_CURRENT_SOURCE_DIR}/*.po")

  foreach(_po_file ${_po_files})
    get_filename_component(_lang "${_po_file}" NAME_WE)

    set(_mo_dir "${CMAKE_CURRENT_BINARY_DIR}/${_lang}/LC_MESSAGES")
    set(_mo_file "${_mo_dir}/gwenhywfar.mo")

    add_custom_command(
      OUTPUT "${_mo_file}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${_mo_dir}"
      COMMAND ${MSGFMT_EXECUTABLE} -o "${_mo_file}" "${_po_file}"
      DEPENDS "${_po_file}"
      COMMENT "Compiling ${_lang}.po -> gwenhywfar.mo"
      VERBATIM
    )

    list(APPEND _mo_files "${_mo_file}")

    install(FILES "${_mo_file}"
      DESTINATION "${CMAKE_INSTALL_DATADIR}/locale/${_lang}/LC_MESSAGES"
    )
  endforeach()

  add_custom_target(i18n ALL DEPENDS ${_mo_files})
endif()
