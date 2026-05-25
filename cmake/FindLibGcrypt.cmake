# cmake/FindLibGcrypt.cmake
# Find the libgcrypt cryptographic library.
#
# Searches via pkg-config first, with a fallback to manual header/library
# search.  The minimum required version defaults to 1.2.0 but can be
# overridden by setting LIBGCRYPT_MIN_VERSION before calling find_package().
#
# Exported variables (set on success):
#   LIBGCRYPT_FOUND        - TRUE if libgcrypt was found
#   LIBGCRYPT_LIBRARIES    - Libraries to link against
#   LIBGCRYPT_INCLUDE_DIRS - Include directories
#   LIBGCRYPT_CFLAGS       - Compiler flags reported by pkg-config (if any)

if(NOT LIBGCRYPT_MIN_VERSION)
  set(LIBGCRYPT_MIN_VERSION "1.2.0")
endif()

# --- Try pkg-config first ---------------------------------------------------
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(_GCRYPT_PC QUIET "libgcrypt >= ${LIBGCRYPT_MIN_VERSION}")
endif()

if(_GCRYPT_PC_FOUND)
  set(LIBGCRYPT_LIBRARIES    "${_GCRYPT_PC_LINK_LIBRARIES}")
  set(LIBGCRYPT_CFLAGS       "${_GCRYPT_PC_CFLAGS}")
  set(LIBGCRYPT_VERSION      "${_GCRYPT_PC_VERSION}")
  # pkg-config omits default system include paths (e.g. /usr/include), so the
  # variable may be empty even when the header is perfectly reachable.
  if(_GCRYPT_PC_INCLUDE_DIRS)
    set(LIBGCRYPT_INCLUDE_DIRS "${_GCRYPT_PC_INCLUDE_DIRS}")
  else()
    find_path(_GCRYPT_FALLBACK_INC NAMES gcrypt.h)
    if(_GCRYPT_FALLBACK_INC)
      set(LIBGCRYPT_INCLUDE_DIRS "${_GCRYPT_FALLBACK_INC}")
    else()
      set(LIBGCRYPT_INCLUDE_DIRS "/usr/include")
    endif()
    mark_as_advanced(_GCRYPT_FALLBACK_INC)
  endif()
else()
  # --- Fallback: manual search -----------------------------------------------
  find_path(LIBGCRYPT_INCLUDE_DIR
    NAMES gcrypt.h
    PATH_SUFFIXES include
  )

  find_library(LIBGCRYPT_LIBRARY
    NAMES gcrypt
    PATH_SUFFIXES lib lib64
  )

  # Try to extract the version from gcrypt.h
  if(LIBGCRYPT_INCLUDE_DIR AND EXISTS "${LIBGCRYPT_INCLUDE_DIR}/gcrypt.h")
    file(STRINGS "${LIBGCRYPT_INCLUDE_DIR}/gcrypt.h" _gcrypt_version_str
      REGEX "^#define[ \t]+GCRYPT_VERSION[ \t]+\"[0-9]+\\.[0-9]+\\.[0-9]+\"")
    if(_gcrypt_version_str)
      string(REGEX REPLACE "^.*\"([0-9]+\\.[0-9]+\\.[0-9]+)\".*$" "\\1"
        LIBGCRYPT_VERSION "${_gcrypt_version_str}")
    endif()
  endif()

  set(LIBGCRYPT_INCLUDE_DIRS "${LIBGCRYPT_INCLUDE_DIR}")
  set(LIBGCRYPT_LIBRARIES    "${LIBGCRYPT_LIBRARY}")
  set(LIBGCRYPT_CFLAGS       "")
endif()

# --- Standard find_package handling ------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibGcrypt
  REQUIRED_VARS LIBGCRYPT_LIBRARIES LIBGCRYPT_INCLUDE_DIRS
  VERSION_VAR   LIBGCRYPT_VERSION
)

mark_as_advanced(LIBGCRYPT_INCLUDE_DIR LIBGCRYPT_LIBRARY)
