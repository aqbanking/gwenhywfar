# plugins/dbio/CMakeLists.txt
# DBIO plugin category — builds selected DBIO plugins.

foreach(_dbio_plugin ${WITH_PLUGINS_DBIO})
  if(_dbio_plugin STREQUAL "csv" OR
     _dbio_plugin STREQUAL "olddb" OR
     _dbio_plugin STREQUAL "xmldb")
    add_subdirectory(${_dbio_plugin})
  else()
    message(FATAL_ERROR "Unknown DBIO plugin: ${_dbio_plugin}")
  endif()
endforeach()
