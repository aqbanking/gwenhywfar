# cmake/GwenMacros.cmake
# Helper macros and functions for the Gwenhywfar CMake build system.
#
# Provides:
#   GWEN_NOINSTALL_HEADERS          - list of headers excluded from installation
#   gwen_convert_autotools_template(INPUT_FILE OUTPUT_FILE)
#   gwen_add_plugin(name category sources...)
#   gwen_install_headers(target dir headers...)
#   gwen_generate_pc_file(template output)

# ---------------------------------------------------------------------------
# Headers that should never be installed (internal implementation files).
# Used by gwen_install_headers() and the symlink directory creation.
# ---------------------------------------------------------------------------
set(GWEN_NOINSTALL_HEADERS
  binreloc.h
  sort_r.h
  cttest.h
  xmlcmd_lxml.h
  xmlcmd_lxml_todb.h
)

# ---------------------------------------------------------------------------
# gwen_convert_autotools_template(INPUT_FILE OUTPUT_FILE)
#
# Reads an autotools .in template from the source tree, maps well-known
# autotools variable names to their CMake equivalents while keeping the
# @variable@ syntax so that configure_file(@ONLY) can substitute them.
#
# Conversion rules:
#   1. A mapping table translates well-known autotools variable names to
#      their CMake equivalents (e.g. @prefix@ -> @CMAKE_INSTALL_PREFIX@).
#   2. Variables whose autotools name already matches the CMake variable
#      name are left untouched (e.g. @gwenhywfar_sys_is_windows@ stays).
#   3. The catch-all leaves any remaining @VARIABLE@ patterns as-is —
#      the caller must ensure those CMake variables are set before calling
#      configure_file(@ONLY).
# ---------------------------------------------------------------------------
function(gwen_convert_autotools_template INPUT_FILE OUTPUT_FILE)
  # Read the autotools template
  file(READ "${INPUT_FILE}" _content)

  # --- Mapping of well-known autotools substitution variables to CMake ---
  # Only remap variables whose autotools name differs from the CMake name.
  # Variables with identical names in both systems are left untouched.

  # Install directories — use cascading ${prefix} references matching autotools
  # behavior so that cmake config files can relocate by changing prefix.
  string(REPLACE "@prefix@"        "@CMAKE_INSTALL_PREFIX@" _content "${_content}")
  string(REPLACE "@exec_prefix@"   "\${prefix}" _content "${_content}")
  string(REPLACE "@libdir@"        "\${exec_prefix}/@CMAKE_INSTALL_LIBDIR@" _content "${_content}")
  string(REPLACE "@includedir@"    "\${prefix}/@CMAKE_INSTALL_INCLUDEDIR@" _content "${_content}")
  string(REPLACE "@bindir@"        "\${exec_prefix}/@CMAKE_INSTALL_BINDIR@" _content "${_content}")
  string(REPLACE "@datarootdir@"   "\${prefix}/@CMAKE_INSTALL_DATAROOTDIR@" _content "${_content}")
  string(REPLACE "@datadir@"       "\${prefix}/@CMAKE_INSTALL_DATADIR@" _content "${_content}")
  string(REPLACE "@sysconfdir@"    "\${prefix}/@CMAKE_INSTALL_SYSCONFDIR@" _content "${_content}")

  # Project version
  string(REPLACE "@VERSION@"       "@GWENHYWFAR_VERSION_STRING@" _content "${_content}")
  string(REPLACE "@PACKAGE@"       "gwenhywfar" _content "${_content}")

  # Gwenhywfar-specific variables — these already match CMake variable names,
  # so they are left as @variable@ for configure_file(@ONLY) to substitute.
  # (No replacement needed for: gwenhywfar_headerdir, gwenhywfar_plugindir,
  #  gwenhywfar_ldflags, gwenhywfar_libs, gwenhywfar_datadir, etc.)

  # Remap gwenhywfar_bindir to the CMake equivalent
  string(REPLACE "@gwenhywfar_bindir@" "@CMAKE_INSTALL_FULL_BINDIR@" _content "${_content}")

  # Autotools build variables that have no CMake equivalent — remove them
  string(REPLACE "@all_includes@"  "" _content "${_content}")

  # Doxygen-related remappings
  string(REPLACE "@top_srcdir@"    "@CMAKE_SOURCE_DIR@" _content "${_content}")
  string(REPLACE "@top_builddir@"  "@CMAKE_BINARY_DIR@" _content "${_content}")
  string(REPLACE "@srcdir@"        "@CMAKE_CURRENT_SOURCE_DIR@" _content "${_content}")
  string(REPLACE "@builddir@"      "@CMAKE_CURRENT_BINARY_DIR@" _content "${_content}")

  # All remaining @VARIABLE@ patterns are left as-is — they must match
  # CMake variable names that are set before configure_file(@ONLY) is called.

  # Ensure parent directory exists
  get_filename_component(_output_dir "${OUTPUT_FILE}" DIRECTORY)
  file(MAKE_DIRECTORY "${_output_dir}")

  # Write the converted template to the build tree
  file(WRITE "${OUTPUT_FILE}" "${_content}")
endfunction()


# ---------------------------------------------------------------------------
# gwen_add_plugin(name category source1 [source2 ...])
#
# Creates a MODULE library for a Gwenhywfar plugin.
#
# Arguments:
#   name      - Plugin target name (e.g. "csv", "ohbci", "cfgdir")
#   category  - Plugin category directory (e.g. "dbio", "ct", "configmgr")
#   source... - Source files for the plugin
#
# Behavior:
#   - Creates a MODULE library with PREFIX "" (no "lib" prefix)
#   - Links against gwenhywfar
#   - Installs the plugin .so/.dll to
#       <libdir>/gwenhywfar/plugins/<SO_EFFECTIVE>/<category>/
#   - If an XML descriptor (<name>.xml or <name>.xml.in) exists in the
#     current source directory, converts (if .in) and installs it alongside
#     the plugin
#   - On Windows, if a ressource.rc.in exists, converts and compiles it
# ---------------------------------------------------------------------------
macro(gwen_add_plugin _plugin_name _plugin_category)
  # Remaining arguments are source files
  set(_plugin_sources ${ARGN})

  # Create the MODULE library
  add_library(${_plugin_name} MODULE ${_plugin_sources})
  set_target_properties(${_plugin_name} PROPERTIES PREFIX "")
  target_link_libraries(${_plugin_name} PRIVATE gwenhywfar)

  target_include_directories(${_plugin_name} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/base
    ${CMAKE_SOURCE_DIR}/src/os
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_BINARY_DIR}  # for config.h
  )

  # Plugin install directory
  set(_plugin_install_dir "${CMAKE_INSTALL_LIBDIR}/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}/${_plugin_category}")

  install(TARGETS ${_plugin_name}
    LIBRARY DESTINATION "${_plugin_install_dir}"
    RUNTIME DESTINATION "${_plugin_install_dir}"  # Windows .dll
  )

  # Handle XML descriptor
  # Check for pre-generated .xml first, then .xml.in template
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin_name}.xml.in")
    gwen_convert_autotools_template(
      "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin_name}.xml.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${_plugin_name}.xml.in"
    )
    configure_file(
      "${CMAKE_CURRENT_BINARY_DIR}/${_plugin_name}.xml.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${_plugin_name}.xml"
      @ONLY
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${_plugin_name}.xml"
      DESTINATION "${_plugin_install_dir}"
    )
  elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin_name}.xml")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${_plugin_name}.xml"
      DESTINATION "${_plugin_install_dir}"
    )
  endif()

  # Handle Windows resource file
  if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ressource.rc.in")
    gwen_convert_autotools_template(
      "${CMAKE_CURRENT_SOURCE_DIR}/ressource.rc.in"
      "${CMAKE_CURRENT_BINARY_DIR}/ressource.rc.in"
    )
    configure_file(
      "${CMAKE_CURRENT_BINARY_DIR}/ressource.rc.in"
      "${CMAKE_CURRENT_BINARY_DIR}/ressource.rc"
      @ONLY
    )
    target_sources(${_plugin_name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/ressource.rc")
  endif()
endmacro()


# ---------------------------------------------------------------------------
# gwen_install_headers(target dir header1 [header2 ...])
#
# Installs public headers, automatically excluding private (*_p.h) and
# local (*_l.h) headers.
#
# Arguments:
#   target  - Unused (reserved for future per-target header sets)
#   dir     - Subdirectory under <prefix>/include/gwenhywfar5/ to install into
#             (e.g. "gwenhywfar", "gwen-gui-cpp", "gwen-gui-qt5")
#   header... - Header files to consider for installation
# ---------------------------------------------------------------------------
macro(gwen_install_headers _target _dir)
  set(_public_headers "")
  foreach(_hdr ${ARGN})
    get_filename_component(_hdr_name "${_hdr}" NAME)
    # Exclude private (*_p.h), local (*_l.h), and test (*-t.h) headers
    string(REGEX MATCH "_p\\.h$" _is_private "${_hdr_name}")
    string(REGEX MATCH "_l\\.h$" _is_local "${_hdr_name}")
    string(REGEX MATCH "-t\\.h$" _is_test "${_hdr_name}")
    if(NOT _is_private AND NOT _is_local AND NOT _is_test
       AND NOT _hdr_name IN_LIST GWEN_NOINSTALL_HEADERS)
      list(APPEND _public_headers "${_hdr}")
    endif()
  endforeach()

  if(_public_headers)
    install(FILES ${_public_headers}
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/${_dir}"
    )
  endif()
endmacro()


# ---------------------------------------------------------------------------
# gwen_generate_pc_file(template output)
#
# Converts an autotools .pc.in template and processes it with
# configure_file() to produce the final .pc file.
#
# Arguments:
#   template - Path to the autotools .pc.in file in the source tree
#   output   - Filename of the final .pc file (e.g. "gwenhywfar.pc")
#
# The converted template is placed in CMAKE_CURRENT_BINARY_DIR and then
# processed by configure_file(@ONLY) to produce the final output.
# The final .pc file is installed to <libdir>/pkgconfig/.
# ---------------------------------------------------------------------------
macro(gwen_generate_pc_file _pc_template _pc_output)
  get_filename_component(_pc_template_name "${_pc_template}" NAME)

  gwen_convert_autotools_template(
    "${_pc_template}"
    "${CMAKE_CURRENT_BINARY_DIR}/${_pc_template_name}"
  )
  configure_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${_pc_template_name}"
    "${CMAKE_CURRENT_BINARY_DIR}/${_pc_output}"
    @ONLY
  )
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${_pc_output}"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
  )
endmacro()
