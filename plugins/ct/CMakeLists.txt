# plugins/ct/CMakeLists.txt
# CryptoToken plugin category — builds selected CT plugins.

foreach(_ct_plugin ${WITH_PLUGINS_CT})
  if(_ct_plugin STREQUAL "ohbci")
    add_subdirectory(${_ct_plugin})
  else()
    message(FATAL_ERROR "Unknown CryptoToken plugin: ${_ct_plugin}")
  endif()
endforeach()
