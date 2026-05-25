# src/os — OS abstraction layer
# Common (platform-independent) sources
add_library(gwen_os OBJECT
  directory_all.c
  gwentime_all.c
  gwendate.c
  process_all.c
  timestamp.c
)

target_include_directories(gwen_os PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/parser
)

target_compile_definitions(gwen_os PRIVATE BUILDING_GWENHYWFAR)

# Conditionally include the platform-specific implementation
if(GWEN_OS STREQUAL "posix")
  add_subdirectory(posix)
elseif(GWEN_OS STREQUAL "windows")
  add_subdirectory(windows)
endif()
