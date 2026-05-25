# ===========================================================================
# Gwenhywfar — Top-level CMakeLists.txt
# ===========================================================================
cmake_minimum_required(VERSION 3.28)

project(gwenhywfar
  VERSION   5.14.1.0
  LANGUAGES C CXX
)

# Enable Objective-C on macOS (needed for Cocoa GUI backend)
if(APPLE)
  enable_language(OBJC)
endif()

# Include helper macros
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GNUInstallDirs)
include(GwenMacros)

# OBJECT libraries aggregated into shared libraries need -fPIC on x86_64.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Global compile definitions matching autotools behavior.
# HAVE_CONFIG_H: source files use #ifdef HAVE_CONFIG_H / #include <config.h>
# PACKAGE: autotools defines this globally via AC_INIT
add_definitions(-DHAVE_CONFIG_H -DPACKAGE="gwenhywfar")

# ===========================================================================
# Version metadata
# ===========================================================================
set(GWENHYWFAR_VERSION_MAJOR    ${PROJECT_VERSION_MAJOR})
set(GWENHYWFAR_VERSION_MINOR    ${PROJECT_VERSION_MINOR})
set(GWENHYWFAR_VERSION_PATCHLEVEL ${PROJECT_VERSION_PATCH})
set(GWENHYWFAR_VERSION_BUILD    ${PROJECT_VERSION_TWEAK})
set(GWENHYWFAR_VERSION_TAG      "stable")
set(GWENHYWFAR_VERSION_STRING   "${GWENHYWFAR_VERSION_MAJOR}.${GWENHYWFAR_VERSION_MINOR}.${GWENHYWFAR_VERSION_PATCHLEVEL}")
set(GWENHYWFAR_VERSION_FULL_STRING
  "${GWENHYWFAR_VERSION_STRING}${GWENHYWFAR_VERSION_TAG}-${GWENHYWFAR_VERSION_BUILD}")

# Release string (matches autotools logic)
if(GWENHYWFAR_VERSION_TAG STREQUAL "cvs" OR
   GWENHYWFAR_VERSION_TAG STREQUAL "svn" OR
   GWENHYWFAR_VERSION_TAG STREQUAL "git")
  set(GWENHYWFAR_VERSION_RELEASE_STRING
    "${GWENHYWFAR_VERSION_MAJOR}.${GWENHYWFAR_VERSION_MINOR}.${GWENHYWFAR_VERSION_PATCHLEVEL}.${GWENHYWFAR_VERSION_BUILD}")
else()
  set(GWENHYWFAR_VERSION_RELEASE_STRING "${GWENHYWFAR_VERSION_STRING}")
endif()

# OHBCI plugin version
set(OHBCI_VERSION_STRING "0.1.0")

# ===========================================================================
# SO version variables
# ===========================================================================

# Gwenhywfar core library
set(GWENHYWFAR_SO_CURRENT   93)
set(GWENHYWFAR_SO_REVISION  1)
set(GWENHYWFAR_SO_AGE       14)
math(EXPR GWENHYWFAR_SO_EFFECTIVE "${GWENHYWFAR_SO_CURRENT} - ${GWENHYWFAR_SO_AGE}")

# GwenUI (GUI backend libraries)
set(GWENUI_SO_CURRENT   1)
set(GWENUI_SO_REVISION  1)
set(GWENUI_SO_AGE       0)
math(EXPR GWENUI_SO_EFFECTIVE "${GWENUI_SO_CURRENT} - ${GWENUI_SO_AGE}")

# GwenPP (C++ wrapper)
set(GWENPP_SO_CURRENT   16)
set(GWENPP_SO_REVISION  0)
set(GWENPP_SO_AGE       0)
math(EXPR GWENPP_SO_EFFECTIVE "${GWENPP_SO_CURRENT} - ${GWENPP_SO_AGE}")

# ===========================================================================
# User-facing CMake options
# ===========================================================================

# GUI backends — semicolon-separated list (cpp is always included)
set(GWEN_GUI_BACKENDS "cpp" CACHE STRING
  "Semicolon-separated list of GUI backends to build (cpp;qt4;qt5;qt6;fox16;gtk2;gtk3;cocoa)")

# Subproject mode
option(GWEN_SUBPROJECT       "Build Gwenhywfar as a subproject"                OFF)

# Certificate handling
option(ENABLE_SYSTEM_CERTS   "Use system-wide TLS certificates"                ON)

# Test code
option(ENABLE_TESTCODE       "Compile test code"                               OFF)

# Local installation mode
option(ENABLE_LOCAL_INSTALL  "Enable local installation mode"                  OFF)

# Profiling
option(ENABLE_PROFILING      "Add -pg profiling flags"                         OFF)

# Memory trace
option(ENABLE_MEMTRACE       "Define GWEN_MEMORY_DEBUG for memory debugging"   OFF)

# Treat warnings as errors
option(ENABLE_ERROR_ON_WARNING "Treat compiler warnings as errors (-Werror / /WX)" OFF)

# Release mode (strip binaries)
option(ENABLE_RELEASE        "Strip installed binaries"                        OFF)

# Documentation
option(ENABLE_FULL_DOC       "Generate Doxygen API documentation"             OFF)

# Network-dependent tests
option(ENABLE_NETWORK_CHECKS "Run network-dependent tests during ctest"        ON)

# Variadic macros
option(ENABLE_VARIADIC_MACROS "Enable variadic macros"                         ON)

# libxml2 support
option(WITH_LIBXML2_CODE     "Compile libxml2-dependent code"                  OFF)

# Buffer hard limit
set(BUFFER_HARDLIMIT "(16*1024*1024)" CACHE STRING
  "Default buffer hard limit expression")

# Plugin selections
set(WITH_PLUGINS_CT    "ohbci"         CACHE STRING "CryptoToken plugins to build")
set(WITH_PLUGINS_DBIO  "csv;olddb;xmldb" CACHE STRING "DBIO plugins to build")
set(WITH_PLUGINS_CFGMGR "dir"         CACHE STRING "ConfigManager plugins to build")

# Search directory overrides
set(GWEN_PLUGIN_SEARCHDIR  "" CACHE PATH "Override plugin search directory")
set(GWEN_DATA_SEARCHDIR    "" CACHE PATH "Override data search directory")
set(GWEN_SYSDATA_SEARCHDIR "" CACHE PATH "Override system data search directory")
set(GWEN_LOCALE_SEARCHDIR  "" CACHE PATH "Override locale search directory")
set(GWEN_CFG_SEARCHDIR     "" CACHE PATH "Override config search directory")

# Required libgcrypt version
set(NEED_LIBGCRYPT_VERSION "1.2.0")

# ===========================================================================
# Derived path variables (used by template conversion and configure_file)
# ===========================================================================
# Cascading path variables for .pc and cmake config templates (use ${prefix} references)
set(gwenhywfar_headerdir   "\${includedir}/gwenhywfar5")
set(gwenhywfar_plugindir   "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}")
set(gwenhywfar_ldflags     "-L\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
set(gwenhywfar_libs        "-lgwenhywfar")
set(gwenhywfar_datadir     "\${prefix}/${CMAKE_INSTALL_DATADIR}/gwenhywfar")
set(gwenhywfar_sysdatadir  "\${prefix}/${CMAKE_INSTALL_DATADIR}")

# Absolute path variables for compile-time defines (PLUGINDIR, GWEN_DATADIR, etc.)
set(gwenhywfar_plugindir_abs   "${CMAKE_INSTALL_FULL_LIBDIR}/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}")
set(gwenhywfar_datadir_abs     "${CMAKE_INSTALL_FULL_DATADIR}/gwenhywfar")
set(gwenhywfar_sysdatadir_abs  "${CMAKE_INSTALL_FULL_DATADIR}")

# Search directory defaults (match autotools logic: local/windows use relative,
# otherwise use full install paths)
if(GWEN_PLUGIN_SEARCHDIR)
  set(gwenhywfar_plugin_searchdir "${GWEN_PLUGIN_SEARCHDIR}")
elseif(WIN32 OR ENABLE_LOCAL_INSTALL)
  set(gwenhywfar_plugin_searchdir "lib/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}")
else()
  set(gwenhywfar_plugin_searchdir "${gwenhywfar_plugindir_abs}")
endif()

if(GWEN_DATA_SEARCHDIR)
  set(gwenhywfar_data_searchdir "${GWEN_DATA_SEARCHDIR}")
elseif(WIN32 OR ENABLE_LOCAL_INSTALL)
  set(gwenhywfar_data_searchdir "share/gwenhywfar")
else()
  set(gwenhywfar_data_searchdir "${gwenhywfar_datadir_abs}")
endif()

if(GWEN_SYSDATA_SEARCHDIR)
  set(gwenhywfar_sysdata_searchdir "${GWEN_SYSDATA_SEARCHDIR}")
elseif(WIN32 OR ENABLE_LOCAL_INSTALL)
  set(gwenhywfar_sysdata_searchdir "share")
else()
  set(gwenhywfar_sysdata_searchdir "${gwenhywfar_sysdatadir_abs}")
endif()

if(GWEN_LOCALE_SEARCHDIR)
  set(gwenhywfar_locale_searchdir "${GWEN_LOCALE_SEARCHDIR}")
elseif(WIN32 OR ENABLE_LOCAL_INSTALL)
  set(gwenhywfar_locale_searchdir "share/locale")
else()
  set(gwenhywfar_locale_searchdir "${CMAKE_INSTALL_FULL_LOCALEDIR}")
endif()

if(GWEN_CFG_SEARCHDIR)
  set(gwenhywfar_cfg_searchdir "${GWEN_CFG_SEARCHDIR}")
elseif(WIN32 OR ENABLE_LOCAL_INSTALL)
  set(gwenhywfar_cfg_searchdir "etc")
else()
  set(gwenhywfar_cfg_searchdir "${CMAKE_INSTALL_FULL_SYSCONFDIR}")
endif()

# ===========================================================================
# 2.2: Platform and compiler detection
# ===========================================================================

# --- OS detection ---
if(WIN32)
  set(GWEN_OS "windows")
  set(gwenhywfar_sys_is_windows 1)
else()
  set(GWEN_OS "posix")
  set(gwenhywfar_sys_is_windows 0)
endif()

message(STATUS "Detected OS type: ${GWEN_OS}")

# --- Compiler warning flags ---
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
  # GCC / Clang: enable warnings by default
  add_compile_options(-Wall)

  # GCC >= 3.4: warn on declaration after statement (C90 compat)
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL "3.4")
      add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wdeclaration-after-statement>)
    endif()
  endif()

  # -Werror
  if(ENABLE_ERROR_ON_WARNING)
    add_compile_options(-Werror)
  endif()

elseif(MSVC)
  # MSVC: C11 standard
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED ON)

  # Suppress MSVC deprecation warnings
  add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE
  )

  # /WX (treat warnings as errors)
  if(ENABLE_ERROR_ON_WARNING)
    add_compile_options(/WX)
  endif()
endif()

# --- Symbol visibility (GCC/Clang) ---
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
  include(CheckCSourceCompiles)
  set(CMAKE_REQUIRED_FLAGS "-Werror -fvisibility=hidden")
  check_c_source_compiles("
    int __attribute__((visibility(\"hidden\"))) foo(void) { return 1; }
    int main(void) { return 0; }
  " GCC_HAS_VISIBILITY)
  unset(CMAKE_REQUIRED_FLAGS)

  if(GCC_HAS_VISIBILITY)
    set(GCC_WITH_VISIBILITY_ATTRIBUTE 1)
    set(VISIBILITY_CFLAGS "-fvisibility=hidden")
  endif()
endif()

# --- MSVC export/import macros ---
# (Handled per-target via BUILDING_GWENHYWFAR define; MSVC uses
#  __declspec(dllexport/dllimport) which the source already guards with
#  #ifdef _MSC_VER or BUILDING_GWENHYWFAR.)

# --- Platform libraries ---
if(WIN32)
  if(MSVC)
    # MSVC: ws2_32 for sockets, advapi32 for system functions
    link_libraries(ws2_32 advapi32)
  elseif(MINGW)
    # MinGW: wsock32
    link_libraries(wsock32)
  endif()
endif()

# --- Profiling ---
set(PROFILING_CFLAGS "")
set(PROFILING_LDFLAGS "")
if(ENABLE_PROFILING)
  set(PROFILING_CFLAGS "-pg")
  set(PROFILING_LDFLAGS "-pg")
  add_compile_options(-pg)
  add_link_options(-pg)
endif()

# --- Release stripping ---
if(ENABLE_RELEASE)
  set(CMAKE_INSTALL_DO_STRIP TRUE)
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_link_options(-s)
  endif()
endif()

# --- Debian multiarch path detection ---
set(DEBIAN_MULTIARCH_DIRS "")
if(UNIX AND NOT APPLE)
  find_program(DPKG_ARCHITECTURE dpkg-architecture)
  if(DPKG_ARCHITECTURE)
    execute_process(
      COMMAND ${DPKG_ARCHITECTURE} -qDEB_HOST_MULTIARCH
      OUTPUT_VARIABLE _multiarch_triplet
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
      RESULT_VARIABLE _dpkg_result
    )
    if(NOT _dpkg_result EQUAL 0 OR _multiarch_triplet STREQUAL "")
      execute_process(
        COMMAND ${DPKG_ARCHITECTURE} -qDEB_HOST_GNU_TYPE
        OUTPUT_VARIABLE _multiarch_triplet
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )
    endif()
    if(NOT _multiarch_triplet STREQUAL "")
      set(DEBIAN_MULTIARCH_DIRS "/lib/${_multiarch_triplet};/usr/lib/${_multiarch_triplet}")
      list(APPEND CMAKE_LIBRARY_PATH ${DEBIAN_MULTIARCH_DIRS})
      message(STATUS "Debian multiarch paths: ${DEBIAN_MULTIARCH_DIRS}")
    else()
      message(STATUS "Debian multiarch paths: not found")
    endif()
    unset(_multiarch_triplet)
    unset(_dpkg_result)
  endif()
endif()

# --- Binary relocation (binreloc) detection ---
set(ENABLE_BINRELOC OFF)
set(BR_PTHREAD 0)
if(UNIX)
  # binreloc requires /proc/self/maps and standard prefix layout
  if(EXISTS "/proc/self/maps")
    set(ENABLE_BINRELOC ON)
    message(STATUS "Binary relocation (binreloc): enabled")
  else()
    message(STATUS "Binary relocation (binreloc): disabled (/proc/self/maps not found)")
  endif()

  if(ENABLE_BINRELOC)
    # Check for pthread support for binreloc
    find_package(Threads)
    if(CMAKE_USE_PTHREADS_INIT)
      set(BR_PTHREAD 1)
    endif()
  endif()
else()
  message(STATUS "Binary relocation (binreloc): disabled (non-UNIX platform)")
endif()

# --- gwenbuild tool (POSIX only) ---
if(GWEN_OS STREQUAL "posix")
  set(gwenbuild_tool "gwenbuild")
else()
  set(gwenbuild_tool "")
endif()

# ===========================================================================
# 2.3: Dependency detection
# ===========================================================================

include(CMakePushCheckState)
find_package(PkgConfig QUIET)

# --- libgcrypt (required, >= 1.2.0) -----------------------------------------
set(LIBGCRYPT_MIN_VERSION "${NEED_LIBGCRYPT_VERSION}")
find_package(LibGcrypt REQUIRED)
message(STATUS "Found libgcrypt: ${LIBGCRYPT_LIBRARIES} (version ${LIBGCRYPT_VERSION})")

# --- gpg-error (required) ----------------------------------------------------
find_package(GpgError REQUIRED)
message(STATUS "Found gpg-error: ${GPG_ERROR_LIBRARIES}")

# --- GnuTLS (required, >= 2.9.8) --------------------------------------------
if(PkgConfig_FOUND)
  pkg_check_modules(LIBGNUTLS REQUIRED "gnutls >= 2.9.8")
else()
  find_path(LIBGNUTLS_INCLUDE_DIRS NAMES gnutls/gnutls.h)
  find_library(LIBGNUTLS_LIBRARIES NAMES gnutls)
  if(NOT LIBGNUTLS_INCLUDE_DIRS OR NOT LIBGNUTLS_LIBRARIES)
    message(FATAL_ERROR
      "GnuTLS >= 2.9.8 is required but was not found.\n"
      "Install the gnutls development package or set PKG_CONFIG_PATH.")
  endif()
endif()
message(STATUS "Found GnuTLS: ${LIBGNUTLS_LIBRARIES}")

# --- OpenSSL (optional) ------------------------------------------------------
find_package(OpenSSL QUIET)
if(OPENSSL_FOUND)
  set(HAVE_OPENSSL 1)
  set(ssl_available "yes")
  message(STATUS "Found OpenSSL: ${OPENSSL_LIBRARIES}")
else()
  set(HAVE_OPENSSL 0)
  set(ssl_available "no")
  message(STATUS "OpenSSL: not found (optional, gct-tool will build without it)")
endif()

# --- pthreads (POSIX) --------------------------------------------------------
if(NOT WIN32)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  if(NOT CMAKE_USE_PTHREADS_INIT)
    message(FATAL_ERROR
      "A pthreads implementation is required on POSIX platforms but was not found.")
  endif()
  message(STATUS "Found pthreads: ${CMAKE_THREAD_LIBS_INIT}")
endif()

# --- iconv -------------------------------------------------------------------
# Some systems have iconv in libc, others need -liconv
include(CheckCSourceCompiles)
set(WITH_ICONV 0)
set(ICONV_LIBS "")

cmake_push_check_state(RESET)
check_c_source_compiles("
  #include <stdlib.h>
  #include <iconv.h>
  int main(void) {
    iconv_t cd = iconv_open(\"\", \"\");
    iconv(cd, NULL, NULL, NULL, NULL);
    return 0;
  }
" ICONV_IN_LIBC)
cmake_pop_check_state()

if(ICONV_IN_LIBC)
  set(WITH_ICONV 1)
  message(STATUS "Found iconv: in libc")
else()
  # Try with -liconv
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_LIBRARIES "iconv")
  check_c_source_compiles("
    #include <stdlib.h>
    #include <iconv.h>
    int main(void) {
      iconv_t cd = iconv_open(\"\", \"\");
      iconv(cd, NULL, NULL, NULL, NULL);
      return 0;
    }
  " ICONV_IN_LIBICONV)
  cmake_pop_check_state()

  if(ICONV_IN_LIBICONV)
    set(WITH_ICONV 1)
    set(ICONV_LIBS "iconv")
    message(STATUS "Found iconv: in libiconv")
  else()
    message(STATUS "iconv: not found")
  endif()
endif()

set(HAVE_ICONV ${WITH_ICONV})

# --- gettext / libintl -------------------------------------------------------
set(HAVE_I18N 0)
set(i18n_libs "")

find_package(Intl QUIET)
if(Intl_FOUND)
  set(HAVE_I18N 1)
  if(Intl_LIBRARIES)
    set(i18n_libs "${Intl_LIBRARIES}")
  endif()
  message(STATUS "Found gettext/libintl: ${Intl_INCLUDE_DIRS}")
else()
  # Fallback: check if gettext functions are in libc
  cmake_push_check_state(RESET)
  check_c_source_compiles("
    #include <libintl.h>
    int main(void) {
      bindtextdomain(\"test\", \"/usr/share/locale\");
      return 0;
    }
  " INTL_IN_LIBC)
  cmake_pop_check_state()

  if(INTL_IN_LIBC)
    set(HAVE_I18N 1)
    message(STATUS "Found gettext/libintl: in libc")
  else()
    # Try with -lintl
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_LIBRARIES "intl")
    check_c_source_compiles("
      #include <libintl.h>
      int main(void) {
        bindtextdomain(\"test\", \"/usr/share/locale\");
        return 0;
      }
    " INTL_IN_LIBINTL)
    cmake_pop_check_state()

    if(INTL_IN_LIBINTL)
      set(HAVE_I18N 1)
      set(i18n_libs "intl")
      message(STATUS "Found gettext/libintl: in libintl")
    else()
      message(STATUS "gettext/libintl: not found (i18n support disabled)")
    endif()
  endif()
endif()

# --- dlopen ------------------------------------------------------------------
set(LIBLD_LIBS "")

include(CheckFunctionExists)
check_function_exists(dlopen DLOPEN_IN_LIBC)
if(DLOPEN_IN_LIBC)
  message(STATUS "Found dlopen: in libc")
else()
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_LIBRARIES "dl")
  check_function_exists(dlopen DLOPEN_IN_LIBDL)
  cmake_pop_check_state()

  if(DLOPEN_IN_LIBDL)
    set(LIBLD_LIBS "dl")
    message(STATUS "Found dlopen: in libdl")
  else()
    if(NOT WIN32)
      message(STATUS "dlopen: not found (dynamic loading may not work)")
    endif()
  endif()
endif()

# --- libxml2 (optional, controlled by WITH_LIBXML2_CODE) --------------------
if(WITH_LIBXML2_CODE)
  if(PkgConfig_FOUND)
    pkg_check_modules(LIBXML REQUIRED "libxml-2.0")
  else()
    find_package(LibXml2 REQUIRED)
    set(LIBXML_INCLUDE_DIRS "${LIBXML2_INCLUDE_DIRS}")
    set(LIBXML_LIBRARIES "${LIBXML2_LIBRARIES}")
  endif()
  if(NOT LIBXML_FOUND AND NOT LIBXML2_FOUND)
    message(FATAL_ERROR
      "WITH_LIBXML2_CODE is ON but libxml-2.0 was not found.\n"
      "Install the libxml2 development package or disable WITH_LIBXML2_CODE.")
  endif()
  set(GWEN_COMPILE_LIBXML2_CODE 1)
  message(STATUS "Found libxml2: ${LIBXML_LIBRARIES}")
endif()

# ===========================================================================
# 2.4: Header and function checks, generate config.h
# ===========================================================================

include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)

# --- Header checks (Requirement 5.5) ---
check_include_file(arpa/inet.h     HAVE_ARPA_INET_H)
check_include_file(fcntl.h         HAVE_FCNTL_H)
check_include_file(netdb.h         HAVE_NETDB_H)
check_include_file(netinet/in.h    HAVE_NETINET_IN_H)
check_include_file(stdlib.h        HAVE_STDLIB_H)
check_include_file(string.h        HAVE_STRING_H)
check_include_file(strings.h       HAVE_STRINGS_H)
check_include_file(sys/socket.h    HAVE_SYS_SOCKET_H)
check_include_file(sys/time.h      HAVE_SYS_TIME_H)
check_include_file(syslog.h        HAVE_SYSLOG_H)
check_include_file(unistd.h        HAVE_UNISTD_H)
check_include_file(dlfcn.h         HAVE_DLFCN_H)
check_include_file(signal.h        HAVE_SIGNAL_H)
check_include_file(termios.h       HAVE_TERMIOS_H)
check_include_file(langinfo.h      HAVE_LANGINFO_H)
check_include_file(localcharset.h  HAVE_LOCALCHARSET_H)
check_include_file(iconv.h         HAVE_ICONV_H)
check_include_file(time.h          HAVE_TIME_H)
check_include_file(locale.h        HAVE_LOCALE_H)
check_include_file(sys/stat.h      HAVE_SYS_STAT_H)
check_include_file(sys/types.h     HAVE_SYS_TYPES_H)
check_include_file(sys/wait.h      HAVE_SYS_WAIT_H)
check_include_file(regex.h         HAVE_REGEX_H)
check_include_file(endian.h        HAVE_ENDIAN_H)
check_include_file(sys/select.h    HAVE_SYS_SELECT_H)

# --- Function checks (Requirement 5.6) ---
check_function_exists(gethostbyaddr  HAVE_GETHOSTBYADDR)
check_function_exists(gethostbyname  HAVE_GETHOSTBYNAME)
check_function_exists(inet_ntoa      HAVE_INET_NTOA)
check_function_exists(isascii        HAVE_ISASCII)
check_function_exists(getpid         HAVE_GETPID)
check_function_exists(memmove        HAVE_MEMMOVE)
check_function_exists(memset         HAVE_MEMSET)
check_function_exists(select         HAVE_SELECT)
check_function_exists(socket         HAVE_SOCKET)
check_function_exists(strcasecmp     HAVE_STRCASECMP)
check_function_exists(strchr         HAVE_STRCHR)
check_function_exists(strdup         HAVE_STRDUP)
check_function_exists(strerror       HAVE_STRERROR)
check_function_exists(snprintf       HAVE_SNPRINTF)
check_function_exists(symlink        HAVE_SYMLINK)
check_function_exists(setlocale      HAVE_SETLOCALE)
check_function_exists(srandom        HAVE_SRANDOM)
check_function_exists(random         HAVE_RANDOM)

# --- Declaration check for sigprocmask (Requirement 5.7) ---
check_symbol_exists(sigprocmask "signal.h" HAVE_DECL_SIGPROCMASK)

# --- Generate config.h ---
# Map CMake options to config.h define variables
if(GWEN_SUBPROJECT)
  set(GWENHYWFAR_IS_SUBPROJECT 1)
endif()
if(ENABLE_SYSTEM_CERTS)
  set(GWEN_TLS_USE_SYSTEM_CERTIFICATES 1)
endif()
if(ENABLE_TESTCODE)
  set(GWENHYWFAR_ENABLE_TESTCODE 1)
endif()
if(NOT ENABLE_NETWORK_CHECKS)
  set(GWENHYWFAR_SKIP_NETWORK_CHECKS 1)
endif()
if(ENABLE_MEMTRACE)
  set(GWEN_MEMORY_DEBUG 1)
endif()
if(NOT ENABLE_VARIADIC_MACROS)
  set(NO_VARIADIC_MACROS 1)
endif()

configure_file(
  "${CMAKE_SOURCE_DIR}/cmake/config.h.in"
  "${CMAKE_BINARY_DIR}/config.h"
  @ONLY
)
include_directories("${CMAKE_BINARY_DIR}")
# The gwenhywfar5/ symlink directory replicates the autotools build-tree layout
# so that #include <gwenhywfar/types.h> and #include <gwen-gui-cpp/cppgui.hpp>
# resolve correctly during the build.
include_directories("${CMAKE_BINARY_DIR}/gwenhywfar5")

# ===========================================================================
# 2.5: Subdirectory inclusion
# ===========================================================================

enable_testing()

# Core library (must come first — other targets link against it)
add_subdirectory(src)

# GUI backends
add_subdirectory(gui)

# Plugins
add_subdirectory(plugins)

# Command-line tools
add_subdirectory(tools)

# Admin tools (mklistdoc)
add_subdirectory(admin)

# Tests
add_subdirectory(test)
add_subdirectory(checks)

# Internationalization
add_subdirectory(po)

# Data files
add_subdirectory(data)

# Documentation
add_subdirectory(doc)

# ===========================================================================
# 12.1: Installation artifacts — pkg-config, cmake config, scripts
# ===========================================================================

# Set PKG_CONFIG variable for the gwenhywfar-config script
find_program(PKG_CONFIG pkg-config)
if(NOT PKG_CONFIG)
  set(PKG_CONFIG "pkg-config")
endif()

# --- gwenhywfar.pc (pkg-config file) ---
gwen_generate_pc_file(
  "${CMAKE_SOURCE_DIR}/gwenhywfar.pc.in"
  "gwenhywfar.pc"
)

# --- gwenhywfar-config.cmake ---
gwen_convert_autotools_template(
  "${CMAKE_SOURCE_DIR}/gwenhywfar-config.cmake.in"
  "${CMAKE_BINARY_DIR}/gwenhywfar-config.cmake.in"
)
configure_file(
  "${CMAKE_BINARY_DIR}/gwenhywfar-config.cmake.in"
  "${CMAKE_BINARY_DIR}/gwenhywfar-config.cmake"
  @ONLY
)
install(FILES "${CMAKE_BINARY_DIR}/gwenhywfar-config.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gwenhywfar-${GWENHYWFAR_VERSION_MAJOR}.${GWENHYWFAR_VERSION_MINOR}"
)

# --- gwenhywfar-config-version.cmake ---
gwen_convert_autotools_template(
  "${CMAKE_SOURCE_DIR}/gwenhywfar-config-version.cmake.in"
  "${CMAKE_BINARY_DIR}/gwenhywfar-config-version.cmake.in"
)
configure_file(
  "${CMAKE_BINARY_DIR}/gwenhywfar-config-version.cmake.in"
  "${CMAKE_BINARY_DIR}/gwenhywfar-config-version.cmake"
  @ONLY
)
install(FILES "${CMAKE_BINARY_DIR}/gwenhywfar-config-version.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gwenhywfar-${GWENHYWFAR_VERSION_MAJOR}.${GWENHYWFAR_VERSION_MINOR}"
)

# --- gwenhywfar-config shell script ---
gwen_convert_autotools_template(
  "${CMAKE_SOURCE_DIR}/gwenhywfar-config.in.in"
  "${CMAKE_BINARY_DIR}/gwenhywfar-config.in"
)
configure_file(
  "${CMAKE_BINARY_DIR}/gwenhywfar-config.in"
  "${CMAKE_BINARY_DIR}/gwenhywfar-config"
  @ONLY
)
install(PROGRAMS "${CMAKE_BINARY_DIR}/gwenhywfar-config"
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# --- gwenhywfar.m4 (aclocal macro) ---
install(FILES "${CMAKE_SOURCE_DIR}/gwenhywfar.m4"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/aclocal"
)

# ===========================================================================
# 12.2: CPack configuration and packaging script generation
# ===========================================================================

# --- Platform-specific packaging scripts ---

# Windows: Inno Setup script
if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/gwenhywfar.iss.in")
  gwen_convert_autotools_template(
    "${CMAKE_SOURCE_DIR}/gwenhywfar.iss.in"
    "${CMAKE_BINARY_DIR}/gwenhywfar.iss.in"
  )
  configure_file(
    "${CMAKE_BINARY_DIR}/gwenhywfar.iss.in"
    "${CMAKE_BINARY_DIR}/gwenhywfar.iss"
    @ONLY
  )
endif()

# Linux: RPM spec file
if(UNIX AND NOT APPLE AND EXISTS "${CMAKE_SOURCE_DIR}/gwenhywfar.spec.in")
  # Set distribution variables used by the spec template (defaults if not set by autotools)
  if(NOT DEFINED aq_distrib_name)
    set(aq_distrib_name "unknown")
  endif()
  if(NOT DEFINED aq_distrib_tag)
    set(aq_distrib_tag "unknown")
  endif()
  if(NOT DEFINED aq_distrib_version)
    set(aq_distrib_version "0")
  endif()

  gwen_convert_autotools_template(
    "${CMAKE_SOURCE_DIR}/gwenhywfar.spec.in"
    "${CMAKE_BINARY_DIR}/gwenhywfar.spec.in"
  )
  configure_file(
    "${CMAKE_BINARY_DIR}/gwenhywfar.spec.in"
    "${CMAKE_BINARY_DIR}/gwenhywfar.spec"
    @ONLY
  )
endif()

# --- CPack configuration ---
set(CPACK_PACKAGE_NAME "gwenhywfar")
set(CPACK_PACKAGE_VERSION "${GWENHYWFAR_VERSION_STRING}")
set(CPACK_PACKAGE_VERSION_MAJOR "${GWENHYWFAR_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${GWENHYWFAR_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${GWENHYWFAR_VERSION_PATCHLEVEL}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A multi-platform helper library for other libraries")
set(CPACK_PACKAGE_VENDOR "Gwenhywfar Development Team")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")

# Generators: TGZ and ZIP at minimum
set(CPACK_GENERATOR "TGZ;ZIP")

# On Windows, optionally add Inno Setup generator
if(WIN32)
  find_program(ISCC_EXECUTABLE NAMES iscc ISCC)
  if(ISCC_EXECUTABLE)
    list(APPEND CPACK_GENERATOR "INNOSETUP")
    set(CPACK_INNOSETUP_SETUP_FILE "${CMAKE_BINARY_DIR}/gwenhywfar.iss")
  endif()
endif()

# Exclude test tools, test dialog data, and gwenbuild from packages
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${PROJECT_NAME};ALL;/")

# Component-based exclusion: mark test programs as excluded from packaging
# These are noinst programs that should never appear in packages
set(_cpack_ignore_files
  "/dlgtest$"
  "/dlgtest\\\\.exe$"
  "/testlib$"
  "/testlib\\\\.exe$"
  "/gwentest$"
  "/gwentest\\\\.exe$"
  "/checks$"
  "/checks\\\\.exe$"
  "/check_urlfns$"
  "/check_urlfns\\\\.exe$"
  "/cttest$"
  "/cttest\\\\.exe$"
  "/testinherit$"
  "/testinherit\\\\.exe$"
  "/libtest$"
  "/libtest\\\\.exe$"
  "/gwen-cocoa-test$"
  "/gwen-cocoa-test\\\\.exe$"
  "/gwenbuild$"
  "/gwenbuild\\\\.exe$"
  "gui/testdialogs/"
)

# Source package exclusions
set(CPACK_SOURCE_IGNORE_FILES
  "/\\\\.git/"
  "/build/"
  "/\\\\.hypothesis/"
  "/\\\\.pytest_cache/"
  "__pycache__"
)

# ===========================================================================
# Configuration summary (matches autotools output)
# ===========================================================================
message("")
message("Summary")
message("=============================================")
message("Version                       : ${GWENHYWFAR_VERSION_FULL_STRING}")
message("System                        : ${CMAKE_SYSTEM_NAME}")
message("Installation Folder           : ${CMAKE_INSTALL_PREFIX}")
message("Local Installation Mode       : ${ENABLE_LOCAL_INSTALL}")
message("Plugin Installation Folder    : ${gwenhywfar_plugindir_abs}")
message("Plugin Search Folder          : ${gwenhywfar_plugin_searchdir}")
message("Data Search Folder            : ${gwenhywfar_data_searchdir}")
message("Locale Search Folder          : ${gwenhywfar_locale_searchdir}")
message("Config Search Folder          : ${gwenhywfar_cfg_searchdir}")
message("Buffer Default Hard Limit     : ${BUFFER_HARDLIMIT}")
if(GCC_WITH_VISIBILITY_ATTRIBUTE)
  message("Symbol Visibility             : yes")
else()
  message("Symbol Visibility             : no")
endif()
message("OpenSSL Support for gct-tool  : ${ssl_available}")
message("GUI Support                   : ${GWEN_GUI_BACKENDS}")
message("Use system-wide certificates  : ${ENABLE_SYSTEM_CERTS}")
message("Compiling test code enabled   : ${ENABLE_TESTCODE}")
message("Compile LibXML2 code          : ${WITH_LIBXML2_CODE}")
message("")
message("Plugins")
message("=============================================")
message("  Crypt token plugins         : ${WITH_PLUGINS_CT}")
message("  DBIO plugins                : ${WITH_PLUGINS_DBIO}")
message("  ConfigManager plugins       : ${WITH_PLUGINS_CFGMGR}")
message("")
message("You can now use 'make' (or 'cmake --build .') to build Gwenhywfar.")
message("")

include(CPack)
