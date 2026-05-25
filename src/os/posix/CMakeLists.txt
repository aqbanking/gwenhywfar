# src/os/posix — POSIX platform implementation (OBJECT library)
add_library(gwen_os_impl OBJECT
  directory.c
  fslock.c
  gwentime.c
  gwenthread.c
  threadlocaldata.c
  inetaddr.c
  inetsocket.c
  init.c
  libloader.c
  process.c
  mutex.c
  semaphore.c
  syncio_file.c
  condvar.c
)

target_include_directories(gwen_os_impl PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/sio
  ${CMAKE_SOURCE_DIR}/src/os
  ${CMAKE_SOURCE_DIR}/src/parser
)

target_compile_definitions(gwen_os_impl PRIVATE BUILDING_GWENHYWFAR)
