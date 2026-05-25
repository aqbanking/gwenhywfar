# ===========================================================================
# tools/CMakeLists.txt — Command-line tools
# ===========================================================================

add_subdirectory(xmlmerge)
add_subdirectory(typemaker)
add_subdirectory(typemaker2)
add_subdirectory(gcttool)
add_subdirectory(gsa)
add_subdirectory(dlgtest)

# gwenbuild is POSIX only
if(GWEN_OS STREQUAL "posix")
  add_subdirectory(gwenbuild)
endif()
