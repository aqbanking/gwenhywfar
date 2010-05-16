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


void Gtk2Gui_Dialog_Extend(GWEN_DIALOG *dlg);
void Gtk2Gui_Dialog_UnExtend(GWEN_DIALOG *dlg);

int Gtk2Gui_Dialog_Setup(GtkWidget *parentWindow);


#endif


