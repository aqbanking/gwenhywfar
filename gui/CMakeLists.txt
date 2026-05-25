# ===========================================================================
# gui/CMakeLists.txt — GUI backend builds
# ===========================================================================

# Valid backend names
set(_valid_backends cpp qt4 qt5 qt6 fox16 gtk2 gtk3 cocoa)

# Resolve the backend list: ensure cpp is always present
set(_backends ${GWEN_GUI_BACKENDS})
list(REMOVE_DUPLICATES _backends)

if(NOT "cpp" IN_LIST _backends)
  list(PREPEND _backends "cpp")
endif()

# Validate entries
foreach(_be ${_backends})
  if(NOT _be IN_LIST _valid_backends)
    message(FATAL_ERROR "Unknown GUI backend: ${_be}")
  endif()
endforeach()

# Qt5 / Qt6 mutual exclusion
if("qt5" IN_LIST _backends AND "qt6" IN_LIST _backends)
  message(FATAL_ERROR "Qt5 and Qt6 cannot be enabled simultaneously")
endif()

# Cocoa is macOS only
if("cocoa" IN_LIST _backends AND NOT APPLE)
  message(FATAL_ERROR "Cocoa backend is only available on macOS")
endif()

# Always build testdialogs and cpp
add_subdirectory(testdialogs)
add_subdirectory(cpp)

# ---------------------------------------------------------------------------
# Create gwenhywfar5/gwen-gui-*/ symlink directories in the build tree
# ---------------------------------------------------------------------------
# Autotools creates $(top_builddir)/gwenhywfar5/gwen-gui-<name>/ with symlinks
# to public headers so that #include <gwen-gui-cpp/cppgui.hpp> works.

# cpp backend (always built)
set(_cpp_symdir "${CMAKE_BINARY_DIR}/gwenhywfar5/gwen-gui-cpp")
file(MAKE_DIRECTORY "${_cpp_symdir}")
file(GLOB _cpp_pub_headers "${CMAKE_CURRENT_SOURCE_DIR}/cpp/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/cpp/*.hpp")
foreach(_hdr ${_cpp_pub_headers})
  get_filename_component(_fname "${_hdr}" NAME)
  string(REGEX MATCH "_p\\.(h|hpp)$" _is_private "${_fname}")
  string(REGEX MATCH "_l\\.(h|hpp)$" _is_local "${_fname}")
  if(NOT _is_private AND NOT _is_local)
    file(CREATE_LINK "${_hdr}" "${_cpp_symdir}/${_fname}" COPY_ON_ERROR SYMBOLIC)
  endif()
endforeach()

# Other GUI backends
foreach(_be ${_backends})
  if(_be STREQUAL "cpp")
    continue()
  endif()
  # Determine source directory (qt6 shares qt5 sources)
  if(_be STREQUAL "qt6")
    set(_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/qt5")
  else()
    set(_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/${_be}")
  endif()
  set(_gui_symdir "${CMAKE_BINARY_DIR}/gwenhywfar5/gwen-gui-${_be}")
  file(MAKE_DIRECTORY "${_gui_symdir}")
  file(GLOB _gui_pub_headers "${_src_dir}/*.h" "${_src_dir}/*.hpp")
  foreach(_hdr ${_gui_pub_headers})
    get_filename_component(_fname "${_hdr}" NAME)
    string(REGEX MATCH "_p\\.(h|hpp)$" _is_private "${_fname}")
    string(REGEX MATCH "_l\\.(h|hpp)$" _is_local "${_fname}")
    if(NOT _is_private AND NOT _is_local)
      file(CREATE_LINK "${_hdr}" "${_gui_symdir}/${_fname}" COPY_ON_ERROR SYMBOLIC)
    endif()
  endforeach()
endforeach()

# Conditionally add each requested backend (cpp already added above)
foreach(_be ${_backends})
  if(_be STREQUAL "cpp")
    continue()  # already added
  elseif(_be STREQUAL "qt6")
    # Qt6 uses the qt5 source directory
    add_subdirectory(qt5)
  else()
    add_subdirectory(${_be})
  endif()
endforeach()
