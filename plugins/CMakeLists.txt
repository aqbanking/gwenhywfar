# plugins/CMakeLists.txt
# Top-level plugin build configuration for Gwenhywfar.
# Conditionally adds plugin categories based on user options.

include(GNUInstallDirs)

# DBIO plugins
if(WITH_PLUGINS_DBIO)
  add_subdirectory(dbio)
endif()

# CryptoToken plugins
if(WITH_PLUGINS_CT)
  add_subdirectory(ct)
endif()

# ConfigManager plugins
if(WITH_PLUGINS_CFGMGR)
  add_subdirectory(configmgr)
endif()
