# src/sio — synchronous I/O sub-module (OBJECT library)
add_library(gwen_sio OBJECT
  syncio.c
  syncio_socket.c
  syncio_memory.c
  syncio_buffered.c
  syncio_tls.c
  syncio_http.c
  ssl_cert_descr.c
  httpsession.c
)

target_include_directories(gwen_sio PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/parser
  ${LIBGNUTLS_INCLUDE_DIRS}
)

target_compile_definitions(gwen_sio PRIVATE BUILDING_GWENHYWFAR)
