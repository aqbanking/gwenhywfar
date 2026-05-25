# src/gui — GUI sub-module (OBJECT library)
add_library(gwen_gui OBJECT
  cgui.c
  cprogress.c
  dialog.c
  gui.c
  nogui.c
  widget.c
  progressdata.c
  dlg_progress.c
  dlg_input.c
  dlg_message.c
  dlg_showbox.c
  passwdstore.c
)

target_include_directories(gwen_gui PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/parser
)

target_compile_definitions(gwen_gui PRIVATE BUILDING_GWENHYWFAR)
