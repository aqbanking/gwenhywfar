# ===========================================================================
# doc/CMakeLists.txt — Documentation
# ===========================================================================

# --- Inheritance example (noinst test) ---
add_subdirectory(inherit)

# --- plugindescr.xsd from autotools template ---
gwen_convert_autotools_template(
  "${CMAKE_CURRENT_SOURCE_DIR}/plugindescr.xsd.in"
  "${CMAKE_CURRENT_BINARY_DIR}/plugindescr.xsd.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/plugindescr.xsd.in"
  "${CMAKE_CURRENT_BINARY_DIR}/plugindescr.xsd"
  @ONLY
)

# --- Doxygen documentation ---
if(ENABLE_FULL_DOC)
  find_package(Doxygen QUIET)
  if(DOXYGEN_FOUND)
    # Set Doxygen-related variables for template substitution
    if(ENABLE_FULL_DOC)
      set(DOXYGEN_INPUT "listdoc.h src gui plugins tools")
      set(DOXYGEN_DEFINE "")
    else()
      set(DOXYGEN_INPUT "listdoc.h gwenhywfar5")
      set(DOXYGEN_DEFINE "DOXYGEN_HIDE")
    endif()

    set(DOXYGEN_GENERATE_QHP "NO")
    set(DOXYGEN_QHG_LOCATION "")
    set(DOXYGEN_QHP_NAMESPACE "de.aquamaniac.gwenhywfar-${GWENHYWFAR_VERSION_STRING}")

    find_program(_qhelpgenerator qhelpgenerator)
    if(NOT _qhelpgenerator)
      find_program(_qhelpgenerator qhelpgenerator-qt5)
    endif()
    if(_qhelpgenerator)
      set(DOXYGEN_QHG_LOCATION "${_qhelpgenerator}")
      set(DOXYGEN_GENERATE_QHP "YES")
    endif()

    # Convert Doxyfile.in from autotools template and generate final Doxyfile
    gwen_convert_autotools_template(
      "${CMAKE_SOURCE_DIR}/Doxyfile.in"
      "${CMAKE_BINARY_DIR}/Doxyfile.in"
    )
    configure_file(
      "${CMAKE_BINARY_DIR}/Doxyfile.in"
      "${CMAKE_BINARY_DIR}/Doxyfile"
      @ONLY
    )

    add_custom_target(doc
      COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/Doxyfile"
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM
    )
  else()
    message(WARNING "Doxygen not found — documentation generation unavailable")
  endif()
endif()
