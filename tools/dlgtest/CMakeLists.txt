# ===========================================================================
# tools/dlgtest/CMakeLists.txt
# ===========================================================================
#
# NOTE: dlgtest references <gwenhywfar/ui/loader.h> and libgwenui, which were
# part of an older UI subsystem that no longer exists in the source tree.
# The autotools build has the same issue (it links against gwenui/libgwenui.la
# which is not built).  Skipping this target until the source is updated.
#
# If the source is fixed in the future, uncomment the following:
#
# add_executable(dlgtest main.c)
# target_include_directories(dlgtest PRIVATE
#   ${CMAKE_SOURCE_DIR}/src
#   ${CMAKE_BINARY_DIR}/src
#   ${CMAKE_BINARY_DIR}
# )
# target_link_libraries(dlgtest PRIVATE gwenhywfar ${i18n_libs})
# if(WIN32 AND MINGW)
#   target_link_options(dlgtest PRIVATE -mconsole)
# endif()
# install(TARGETS dlgtest DESTINATION ${CMAKE_INSTALL_BINDIR})
