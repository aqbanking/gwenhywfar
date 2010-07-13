
#undef BUILDING_QT3_GUI

#include "qt3_gui.hpp"
#include "../testdialogs/dlg_test.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/dialog.h>
#include <gwenhywfar/debug.h>

#include <qapplication.h>


int test1(int argc, char **argv) {
  QApplication a(argc, argv);
  QT3_Gui *gui;
  int rv;
  GWEN_DIALOG *dlg;

  rv=GWEN_Init();
  if (rv) {
    DBG_ERROR_ERR(0, rv);
    return 2;
  }

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);

  /* create GUI */
  gui=new QT3_Gui();
  GWEN_Gui_SetGui(gui->getCInterface());

  dlg=Dlg_Test1_new();
  if (dlg==NULL) {
    fprintf(stderr, "Could not create dialog.\n");
    return 2;
  }

  rv=GWEN_Gui_ExecDialog(dlg, 0);
  fprintf(stderr, "Result: %d\n", rv);

  return 0;
}



int main(int argc, char **argv) {
  return test1(argc, argv);
}





