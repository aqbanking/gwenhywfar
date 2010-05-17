/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GTK2_GUI_DIALOG_L_H
#define GTK2_GUI_DIALOG_L_H


#include <gwen-gui-gtk2/gtk2_gui.h>


#include <gtk/gtk.h>

#include <gwenhywfar/dialog_be.h>


#define GTK2_GUI_DIALOG_DEFAULT_BOX_SPACING 3


typedef struct {
  GWEN_DIALOG *dialog;
  GtkWindow *window;
  int response;
  GMainLoop *loop;
  int destroyed;
} RunInfo;



void Gtk2Gui_Dialog_Extend(GWEN_DIALOG *dlg);
void Gtk2Gui_Dialog_Unextend(GWEN_DIALOG *dlg);

int Gtk2Gui_Dialog_Setup(GWEN_DIALOG *dlg, GtkWidget *parentWindow);


GtkWidget *Gtk2Gui_Dialog_GetMainWidget(const GWEN_DIALOG *dlg);


int GTK2_Gui_Dialog_Run(GWEN_DIALOG *dlg, int timeout);
void Gtk2Gui_Dialog_Leave(GWEN_DIALOG *dlg, int result);

#endif


