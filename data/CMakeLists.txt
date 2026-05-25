# ===========================================================================
# data/CMakeLists.txt — Data files
# ===========================================================================

if(NOT ENABLE_SYSTEM_CERTS)
  install(FILES ca-bundle.crt
    DESTINATION "${CMAKE_INSTALL_DATADIR}/gwenhywfar"
  )
endif()
