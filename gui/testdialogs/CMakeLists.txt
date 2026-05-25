# ===========================================================================
# gui/testdialogs/CMakeLists.txt — Static convenience library for GUI tests
# ===========================================================================

add_library(gwengui-test STATIC
  dlg_test.c
  dlg_test2.c
)

target_include_directories(gwengui-test PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/gwenhywfar5
  ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(gwengui-test PRIVATE
  MEDIAPATH="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(gwengui-test PRIVATE gwenhywfar)

# NOT installed — internal convenience library for GUI backend test programs
