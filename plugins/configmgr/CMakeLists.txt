# plugins/configmgr/CMakeLists.txt
# ConfigManager plugin category — builds selected ConfigManager plugins.

foreach(_cfgmgr_plugin ${WITH_PLUGINS_CFGMGR})
  if(_cfgmgr_plugin STREQUAL "dir")
    add_subdirectory(${_cfgmgr_plugin})
  else()
    message(FATAL_ERROR "Unknown ConfigManager plugin: ${_cfgmgr_plugin}")
  endif()
endforeach()
