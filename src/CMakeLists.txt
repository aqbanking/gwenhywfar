# ===========================================================================
# src/CMakeLists.txt — Core library sub-modules and aggregation
# ===========================================================================

# Common include directories for all sub-modules
include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  # The symlink directory replicates the autotools gwenhywfar5/ structure so
  # that #include <gwenhywfar/types.h> resolves during the build.
  ${CMAKE_BINARY_DIR}/gwenhywfar5
)

# ---------------------------------------------------------------------------
# Generate version.h, types.h, and system.h via template conversion
# ---------------------------------------------------------------------------

# version.h: convert autotools .in template, then substitute variables
gwen_convert_autotools_template(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)

# types.h: convert autotools .in template, then substitute variables
gwen_convert_autotools_template(
  "${CMAKE_CURRENT_SOURCE_DIR}/types.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/types.h.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/types.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/types.h"
  @ONLY
)

# system.h: select platform-appropriate template, convert, then substitute
if(GWEN_OS STREQUAL "windows")
  set(_system_h_template "${CMAKE_CURRENT_SOURCE_DIR}/os/windows/system.h.in")
else()
  set(_system_h_template "${CMAKE_CURRENT_SOURCE_DIR}/os/posix/system.h.in")
endif()

gwen_convert_autotools_template(
  "${_system_h_template}"
  "${CMAKE_CURRENT_BINARY_DIR}/system.h.in"
)
configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/system.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/system.h"
  @ONLY
)

# ---------------------------------------------------------------------------
# Create gwenhywfar5/gwenhywfar/ header directory in the build tree
# ---------------------------------------------------------------------------
# Autotools creates $(top_builddir)/gwenhywfar5/gwenhywfar/ with symlinks to
# all public headers so that #include <gwenhywfar/types.h> works.  We replicate
# this with symlinks (POSIX) or copies (Windows).
set(_symlink_dir "${CMAKE_BINARY_DIR}/gwenhywfar5/gwenhywfar")
file(REMOVE_RECURSE "${_symlink_dir}")
file(MAKE_DIRECTORY "${_symlink_dir}")

# Source directories whose public headers are exposed as <gwenhywfar/*.h>
set(_pub_header_src_dirs
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/base"
  "${CMAKE_CURRENT_SOURCE_DIR}/sio"
  "${CMAKE_CURRENT_SOURCE_DIR}/os"
  "${CMAKE_CURRENT_SOURCE_DIR}/os/${GWEN_OS}"
  "${CMAKE_CURRENT_SOURCE_DIR}/parser"
  "${CMAKE_CURRENT_SOURCE_DIR}/crypt3"
  "${CMAKE_CURRENT_SOURCE_DIR}/crypttoken"
  "${CMAKE_CURRENT_SOURCE_DIR}/gui"
  "${CMAKE_CURRENT_SOURCE_DIR}/cryptmsg"
  "${CMAKE_CURRENT_SOURCE_DIR}/html"
  "${CMAKE_CURRENT_SOURCE_DIR}/sar"
  "${CMAKE_CURRENT_SOURCE_DIR}/xmlcmd"
  "${CMAKE_CURRENT_SOURCE_DIR}/msgio"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_framework"
)

foreach(_dir ${_pub_header_src_dirs})
  file(GLOB _hdrs "${_dir}/*.h")
  foreach(_hdr ${_hdrs})
    get_filename_component(_fname "${_hdr}" NAME)
    # Skip private and local headers from the symlink directory.
    # Note: -t.h (test type) headers ARE included here because source files
    # need them for compilation; they are only excluded from installation.
    string(REGEX MATCH "_p\\.h$" _is_private "${_fname}")
    string(REGEX MATCH "_l\\.h$" _is_local "${_fname}")
    if(NOT _is_private AND NOT _is_local
       AND NOT _fname IN_LIST GWEN_NOINSTALL_HEADERS)
      # Only create if not already present (first directory wins, matching
      # autotools ln -s behavior where duplicate names are silently skipped).
      if(NOT EXISTS "${_symlink_dir}/${_fname}")
        file(CREATE_LINK "${_hdr}" "${_symlink_dir}/${_fname}" COPY_ON_ERROR SYMBOLIC)
      endif()
    endif()
  endforeach()
endforeach()

# Also link the generated headers (version.h, types.h, system.h)
file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/version.h" "${_symlink_dir}/version.h" COPY_ON_ERROR SYMBOLIC)
file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/types.h"   "${_symlink_dir}/types.h"   COPY_ON_ERROR SYMBOLIC)
file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/system.h"  "${_symlink_dir}/system.h"  COPY_ON_ERROR SYMBOLIC)

# ---------------------------------------------------------------------------
# Sub-module OBJECT libraries
# ---------------------------------------------------------------------------
add_subdirectory(base)
add_subdirectory(sio)
add_subdirectory(os)
add_subdirectory(parser)
add_subdirectory(crypt3)
add_subdirectory(crypttoken)
add_subdirectory(gui)
add_subdirectory(cryptmsg)
add_subdirectory(html)
add_subdirectory(sar)
add_subdirectory(xmlcmd)
add_subdirectory(msgio)
add_subdirectory(test_framework)

# ---------------------------------------------------------------------------
# Aggregate all object libraries into the shared library: libgwenhywfar
# ---------------------------------------------------------------------------
add_library(gwenhywfar SHARED
  binreloc.c
  gwenhywfar.c
  $<TARGET_OBJECTS:gwen_base>
  $<TARGET_OBJECTS:gwen_sio>
  $<TARGET_OBJECTS:gwen_os>
  $<TARGET_OBJECTS:gwen_os_impl>
  $<TARGET_OBJECTS:gwen_parser>
  $<TARGET_OBJECTS:gwen_crypt3>
  $<TARGET_OBJECTS:gwen_crypttoken>
  $<TARGET_OBJECTS:gwen_gui>
  $<TARGET_OBJECTS:gwen_cryptmsg>
  $<TARGET_OBJECTS:gwen_html>
  $<TARGET_OBJECTS:gwen_sar>
  $<TARGET_OBJECTS:gwen_xmlcmd>
  $<TARGET_OBJECTS:gwen_msgio>
  $<TARGET_OBJECTS:gwen_test_framework>
)

# ---------------------------------------------------------------------------
# SO version properties (EFFECTIVE = CURRENT - AGE)
# ---------------------------------------------------------------------------
set_target_properties(gwenhywfar PROPERTIES
  VERSION   "${GWENHYWFAR_SO_EFFECTIVE}.${GWENHYWFAR_SO_AGE}.${GWENHYWFAR_SO_REVISION}"
  SOVERSION "${GWENHYWFAR_SO_EFFECTIVE}"
)

# ---------------------------------------------------------------------------
# Include directories for the aggregated library
# ---------------------------------------------------------------------------
target_include_directories(gwenhywfar PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/os
  ${CMAKE_SOURCE_DIR}/src/parser
  ${CMAKE_SOURCE_DIR}/src/gui
  ${LIBGCRYPT_INCLUDE_DIRS}
  ${GPG_ERROR_INCLUDE_DIRS}
  ${LIBGNUTLS_INCLUDE_DIRS}
)

target_include_directories(gwenhywfar PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gwenhywfar5>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/gwenhywfar5/gwenhywfar>
)

# ---------------------------------------------------------------------------
# Compile definitions
# ---------------------------------------------------------------------------
target_compile_definitions(gwenhywfar PRIVATE
  BUILDING_GWENHYWFAR
  GWEN_PREFIX_DIR="${CMAKE_INSTALL_PREFIX}"
  GWEN_SYSCONF_DIR="${gwenhywfar_cfg_searchdir}"
  LOCALEDIR="${gwenhywfar_locale_searchdir}"
  LIBDIR="${CMAKE_INSTALL_FULL_LIBDIR}"
  GWENHYWFAR_SO_EFFECTIVE_STR="${GWENHYWFAR_SO_EFFECTIVE}"
  GWEN_DATADIR="${gwenhywfar_data_searchdir}"
  GWEN_SYSDATADIR="${gwenhywfar_sysdata_searchdir}"
  PLUGINDIR="${gwenhywfar_plugin_searchdir}"
)

# ---------------------------------------------------------------------------
# Symbol visibility (GCC/Clang)
# ---------------------------------------------------------------------------
if(GCC_WITH_VISIBILITY_ATTRIBUTE)
  target_compile_options(gwenhywfar PRIVATE -fvisibility=hidden)
endif()

# ---------------------------------------------------------------------------
# Link dependencies
# ---------------------------------------------------------------------------
target_link_libraries(gwenhywfar PRIVATE
  ${LIBGCRYPT_LIBRARIES}
  ${GPG_ERROR_LIBRARIES}
  ${LIBGNUTLS_LIBRARIES}
)

# pthreads (POSIX)
if(NOT WIN32 AND TARGET Threads::Threads)
  target_link_libraries(gwenhywfar PRIVATE Threads::Threads)
endif()

# iconv
if(ICONV_LIBS)
  target_link_libraries(gwenhywfar PRIVATE ${ICONV_LIBS})
endif()

# gettext / libintl
if(i18n_libs)
  target_link_libraries(gwenhywfar PRIVATE ${i18n_libs})
endif()

# dlopen
if(LIBLD_LIBS)
  target_link_libraries(gwenhywfar PRIVATE ${LIBLD_LIBS})
endif()

# libxml2 (optional)
if(WITH_LIBXML2_CODE AND LIBXML_LIBRARIES)
  target_link_libraries(gwenhywfar PRIVATE ${LIBXML_LIBRARIES})
  target_include_directories(gwenhywfar PRIVATE ${LIBXML_INCLUDE_DIRS})
endif()

# Platform libraries (Windows)
if(WIN32)
  if(MSVC)
    target_link_libraries(gwenhywfar PRIVATE ws2_32 advapi32)
  elseif(MINGW)
    target_link_libraries(gwenhywfar PRIVATE wsock32)
  endif()
endif()

# ---------------------------------------------------------------------------
# Windows resource file
# ---------------------------------------------------------------------------
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
  target_sources(gwenhywfar PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/ressource.rc")
endif()

# ---------------------------------------------------------------------------
# Install core library
# ---------------------------------------------------------------------------
install(TARGETS gwenhywfar
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"  # Windows .dll
)

# ---------------------------------------------------------------------------
# Install public headers (excluding *_p.h and *_l.h)
# ---------------------------------------------------------------------------
# Collect headers from each sub-module directory
set(_header_dirs
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/base"
  "${CMAKE_CURRENT_SOURCE_DIR}/sio"
  "${CMAKE_CURRENT_SOURCE_DIR}/os"
  "${CMAKE_CURRENT_SOURCE_DIR}/parser"
  "${CMAKE_CURRENT_SOURCE_DIR}/crypt3"
  "${CMAKE_CURRENT_SOURCE_DIR}/crypttoken"
  "${CMAKE_CURRENT_SOURCE_DIR}/gui"
  "${CMAKE_CURRENT_SOURCE_DIR}/cryptmsg"
  "${CMAKE_CURRENT_SOURCE_DIR}/html"
  "${CMAKE_CURRENT_SOURCE_DIR}/sar"
  "${CMAKE_CURRENT_SOURCE_DIR}/xmlcmd"
  "${CMAKE_CURRENT_SOURCE_DIR}/msgio"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_framework"
)

set(_all_headers "")
foreach(_dir ${_header_dirs})
  file(GLOB _dir_headers "${_dir}/*.h")
  list(APPEND _all_headers ${_dir_headers})
endforeach()

# Also include .tmpl files from base (list1.tmpl, list2.tmpl)
file(GLOB _tmpl_files "${CMAKE_CURRENT_SOURCE_DIR}/base/*.tmpl")
list(APPEND _all_headers ${_tmpl_files})

# Generated headers from the build tree
list(APPEND _all_headers
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  "${CMAKE_CURRENT_BINARY_DIR}/types.h"
  "${CMAKE_CURRENT_BINARY_DIR}/system.h"
)

gwen_install_headers(gwenhywfar gwenhywfar ${_all_headers})

# ---------------------------------------------------------------------------
# testlib — test executable (NOT installed, registered with CTest)
# ---------------------------------------------------------------------------
add_executable(testlib testlib.c)
target_link_libraries(testlib PRIVATE gwenhywfar)
target_include_directories(testlib PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)
add_test(NAME testlib COMMAND testlib)
