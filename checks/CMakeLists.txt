# ===========================================================================
# checks/CMakeLists.txt — checks and check_urlfns (noinst, NOT installed)
# ===========================================================================

# --- checks executable (network-dependent tests) ---
add_executable(checks
  main.c
  db.c
  syncio_tls.c
  syncio_http.c
  syncio_https.c
)

target_include_directories(checks PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(checks PRIVATE gwenhywfar)

# --- check_urlfns executable ---
add_executable(check_urlfns
  check_urlfns.c
)

target_include_directories(check_urlfns PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(check_urlfns PRIVATE gwenhywfar)

# --- CTest registration ---
# check_urlfns is NOT network-dependent — always register
add_test(NAME check_urlfns COMMAND check_urlfns)

# checks contains network tests (syncio_tls, syncio_http, syncio_https)
# Only register with CTest when ENABLE_NETWORK_CHECKS is ON
if(ENABLE_NETWORK_CHECKS)
  add_test(NAME checks COMMAND checks)
endif()
