# src/xmlcmd — XML command sub-module (OBJECT library)
set(GWEN_XMLCMD_SOURCES
  xmlcmd.c
  xmlcmd_gxml.c
  xmlcmd_gxml_todb.c
  xmlcmd_gxml_fromdb.c
)

# Conditionally add libxml2-dependent sources
if(WITH_LIBXML2_CODE)
  list(APPEND GWEN_XMLCMD_SOURCES
    xmlcmd_lxml.c
    xmlcmd_lxml_todb.c
  )
endif()

add_library(gwen_xmlcmd OBJECT ${GWEN_XMLCMD_SOURCES})

target_include_directories(gwen_xmlcmd PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/os
  ${CMAKE_SOURCE_DIR}/src/parser
)

if(WITH_LIBXML2_CODE)
  target_include_directories(gwen_xmlcmd PRIVATE ${LIBXML_INCLUDE_DIRS})
endif()

target_compile_definitions(gwen_xmlcmd PRIVATE BUILDING_GWENHYWFAR)
