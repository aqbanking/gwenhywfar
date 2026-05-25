# src/base — base sub-module (OBJECT library)
add_library(gwen_base OBJECT
  args.c
  buffer.c
  debug.c
  error.c
  funcs.c
  gwensignal.c
  i18n.c
  idlist64.c
  inherit.c
  list.c
  list1.c
  logger.c
  memory.c
  pathmanager.c
  plugin.c
  plugindescr.c
  refptr.c
  ringbuffer.c
  stringlist.c
  stringlist2.c
  idmap.c
  fastbuffer.c
  memcache.c
  tree.c
  tree2.c
  simpleptrlist.c
  multicache.c
  param.c
  quicksort.c
)

target_include_directories(gwen_base PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/parser
)

target_compile_definitions(gwen_base PRIVATE
  BUILDING_GWENHYWFAR
  GWEN_BUFFER_DEFAULT_HARDLIMIT=${BUFFER_HARDLIMIT}
)
