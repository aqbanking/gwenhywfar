# ===========================================================================
# doc/inherit/CMakeLists.txt — inheritance example (noinst, NOT installed)
# ===========================================================================

# --- Convenience library (not installed) ---
add_library(inherit STATIC
  baseclass.c
  derivedclass.c
)

target_include_directories(inherit PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(inherit PRIVATE gwenhywfar)

# --- testinherit executable (not installed) ---
add_executable(testinherit
  testinherit.c
)

target_include_directories(testinherit PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(testinherit PRIVATE inherit gwenhywfar)

# --- CTest registration ---
add_test(NAME testinherit COMMAND testinherit)
