/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "testdialog.h"

#include "gtk2_gui.h"



int main(int argc, char **argv)
{
  GWEN_GUI *gui;
  GWEN_DIALOG *dlg;
  int rv;

  rv=GWEN_Init();
  if (rv) {
    DBG_ERROR_ERR(0, rv);
    return 2;
  }

  gtk_set_locale();
  gtk_init(&argc, &argv);

  gui=Gtk2_Gui_new();
  GWEN_Gui_SetGui(gui);

  dlg=TestDialog_new();
  assert(dlg);
  rv=GWEN_Gui_ExecDialog(dlg, 0);
  fprintf(stderr, "Result: %d\n", rv);
  return 0;
}


