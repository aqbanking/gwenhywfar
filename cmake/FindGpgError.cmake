# cmake/FindGpgError.cmake
# Find the gpg-error library (libgpg-error).
#
# Searches via pkg-config first, with a fallback to manual header/library
# search.
#
# Exported variables (set on success):
#   GPG_ERROR_FOUND        - TRUE if gpg-error was found
#   GPG_ERROR_LIBRARIES    - Libraries to link against
#   GPG_ERROR_INCLUDE_DIRS - Include directories

# --- Try pkg-config first ---------------------------------------------------
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(_GPG_ERROR_PC QUIET "gpg-error")
endif()

if(_GPG_ERROR_PC_FOUND)
  set(GPG_ERROR_LIBRARIES    "${_GPG_ERROR_PC_LINK_LIBRARIES}")
  set(GPG_ERROR_VERSION      "${_GPG_ERROR_PC_VERSION}")
  # pkg-config omits default system include paths (e.g. /usr/include), so the
  # variable may be empty even when the header is perfectly reachable.
  if(_GPG_ERROR_PC_INCLUDE_DIRS)
    set(GPG_ERROR_INCLUDE_DIRS "${_GPG_ERROR_PC_INCLUDE_DIRS}")
  else()
    # Verify the header is actually available on the default include path.
    find_path(_GPG_ERROR_FALLBACK_INC NAMES gpg-error.h)
    if(_GPG_ERROR_FALLBACK_INC)
      set(GPG_ERROR_INCLUDE_DIRS "${_GPG_ERROR_FALLBACK_INC}")
    else()
      set(GPG_ERROR_INCLUDE_DIRS "/usr/include")
    endif()
    mark_as_advanced(_GPG_ERROR_FALLBACK_INC)
  endif()
else()
  # --- Fallback: manual search -----------------------------------------------
  find_path(GPG_ERROR_INCLUDE_DIR
    NAMES gpg-error.h
    PATH_SUFFIXES include
  )

  find_library(GPG_ERROR_LIBRARY
    NAMES gpg-error
    PATH_SUFFIXES lib lib64
  )

  # Try to extract the version from gpg-error.h
  if(GPG_ERROR_INCLUDE_DIR AND EXISTS "${GPG_ERROR_INCLUDE_DIR}/gpg-error.h")
    file(STRINGS "${GPG_ERROR_INCLUDE_DIR}/gpg-error.h" _gpg_error_version_str
      REGEX "^#define[ \t]+GPG_ERR_VERSION[ \t]+\"[0-9]+\\.[0-9]+\"")
    if(_gpg_error_version_str)
      string(REGEX REPLACE "^.*\"([0-9]+\\.[0-9]+)\".*$" "\\1"
        GPG_ERROR_VERSION "${_gpg_error_version_str}")
    endif()
  endif()

  set(GPG_ERROR_INCLUDE_DIRS "${GPG_ERROR_INCLUDE_DIR}")
  set(GPG_ERROR_LIBRARIES    "${GPG_ERROR_LIBRARY}")
endif()

# --- Standard find_package handling ------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GpgError
  REQUIRED_VARS GPG_ERROR_LIBRARIES GPG_ERROR_INCLUDE_DIRS
  VERSION_VAR   GPG_ERROR_VERSION
)

mark_as_advanced(GPG_ERROR_INCLUDE_DIR GPG_ERROR_LIBRARY)
