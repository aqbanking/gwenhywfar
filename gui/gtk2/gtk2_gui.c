/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gtk2_gui_p.h"
#include "gtk2_gui_dialog_l.h"

#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)



GWEN_INHERIT(GWEN_GUI, GTK2_GUI)



GWEN_GUI *Gtk2_Gui_new() {
  GWEN_GUI *gui;
  GTK2_GUI *xgui;

  gui=GWEN_Gui_new();
  GWEN_NEW_OBJECT(GTK2_GUI, xgui);
  GWEN_INHERIT_SETDATA(GWEN_GUI, GTK2_GUI, gui, xgui, Gtk2_Gui_FreeData);

  GWEN_Gui_UseDialogs(gui);
  xgui->execDialogFn=GWEN_Gui_SetExecDialogFn(gui, GTK2_Gui_ExecDialog);
  xgui->openDialogFn=GWEN_Gui_SetOpenDialogFn(gui, GTK2_Gui_OpenDialog);
  xgui->closeDialogFn=GWEN_Gui_SetCloseDialogFn(gui, GTK2_Gui_CloseDialog);
  xgui->runDialogFn=GWEN_Gui_SetRunDialogFn(gui, GTK2_Gui_RunDialog);

  return gui;
}



void Gtk2_Gui_FreeData(void *bp, void *p) {
  GTK2_GUI *xgui;

  xgui=(GTK2_GUI*) p;

  GWEN_FREE_OBJECT(xgui);
}



int GTK2_Gui_ExecDialog(GWEN_GUI *gui,
			GWEN_DIALOG *dlg,
			uint32_t guiid) {
  int rv;

  rv=GTK2_Gui_OpenDialog(gui, dlg, guiid);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=GTK2_Gui_RunDialog(gui, dlg, 1);
  GTK2_Gui_CloseDialog(gui, dlg);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int GTK2_Gui_OpenDialog(GWEN_GUI *gui,
			GWEN_DIALOG *dlg,
			uint32_t guiid) {
  int rv;
  GtkWidget *g;

  Gtk2Gui_Dialog_Extend(dlg);
  rv=Gtk2Gui_Dialog_Setup(dlg, NULL);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    Gtk2Gui_Dialog_Unextend(dlg);
    return rv;
  }

  g=Gtk2Gui_Dialog_GetMainWidget(dlg);
  if (g==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No main widget");
    Gtk2Gui_Dialog_Unextend(dlg);
    return GWEN_ERROR_INVALID;
  }

  rv=GWEN_Dialog_EmitSignalToAll(dlg, GWEN_DialogEvent_TypeInit, "");
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error initializing dialog: %d", rv);
    Gtk2Gui_Dialog_Unextend(dlg);
    return rv;
  }

  /* show dialog */
  gtk_widget_show_all(g);

  return 0;
}



int GTK2_Gui_CloseDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg) {
  GtkWidget *g;
  int rv;

  g=Gtk2Gui_Dialog_GetMainWidget(dlg);
  if (g==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No main widget");
    Gtk2Gui_Dialog_Unextend(dlg);
    return GWEN_ERROR_INVALID;
  }

  /* hide dialog */
  gtk_widget_hide_all(g);

  /* send fini signal to dialog */
  rv=GWEN_Dialog_EmitSignalToAll(dlg, GWEN_DialogEvent_TypeFini, "");
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error deinitializing dialog: %d", rv);
    Gtk2Gui_Dialog_Unextend(dlg);
    return rv;
  }

  Gtk2Gui_Dialog_Unextend(dlg);
  return 0;
}



int GTK2_Gui_RunDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, int untilEnd) {
  int rv;

  rv=GTK2_Gui_Dialog_Run(dlg, untilEnd);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return rv;
}






