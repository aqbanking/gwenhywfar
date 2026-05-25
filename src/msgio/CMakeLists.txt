# src/msgio — message I/O sub-module (OBJECT library)
add_library(gwen_msgio OBJECT
  endpoint.c
  endpoint_msgio.c
  endpoint_tcpc.c
  endpoint_tcpd.c
  endpoint_ipc.c
  endpoint_multilayer.c
  msg.c
  msg_ipc.c
  request.c
)

target_include_directories(gwen_msgio PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/os
  ${CMAKE_SOURCE_DIR}/src/parser
)

target_compile_definitions(gwen_msgio PRIVATE BUILDING_GWENHYWFAR)
