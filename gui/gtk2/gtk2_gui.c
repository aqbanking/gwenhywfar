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



GWEN_GUI *Gtk2_Gui_new()
{
  GWEN_GUI *gui;
  GTK2_GUI *xgui;

  gui=GWEN_Gui_new();
  GWEN_NEW_OBJECT(GTK2_GUI, xgui);
  GWEN_INHERIT_SETDATA(GWEN_GUI, GTK2_GUI, gui, xgui, Gtk2_Gui_FreeData);

  GWEN_Gui_AddFlags(gui, GWEN_GUI_FLAGS_DIALOGSUPPORTED);
  GWEN_Gui_UseDialogs(gui);
  xgui->execDialogFn=GWEN_Gui_SetExecDialogFn(gui, GTK2_Gui_ExecDialog);
  xgui->openDialogFn=GWEN_Gui_SetOpenDialogFn(gui, GTK2_Gui_OpenDialog);
  xgui->closeDialogFn=GWEN_Gui_SetCloseDialogFn(gui, GTK2_Gui_CloseDialog);
  xgui->runDialogFn=GWEN_Gui_SetRunDialogFn(gui, GTK2_Gui_RunDialog);
  xgui->getFileNameDialogFn=GWEN_Gui_SetGetFileNameFn(gui, GTK2_Gui_GetFileName);

  return gui;
}



GWENHYWFAR_CB void Gtk2_Gui_FreeData(void *bp, void *p)
{
  GTK2_GUI *xgui;

  xgui=(GTK2_GUI *) p;

  GWEN_FREE_OBJECT(xgui);
}



GWENHYWFAR_CB int GTK2_Gui_ExecDialog(GWEN_GUI *gui,
                                      GWEN_DIALOG *dlg,
                                      uint32_t guiid)
{
  int rv;

  assert(dlg);
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



GWENHYWFAR_CB int GTK2_Gui_OpenDialog(GWEN_GUI *gui,
                                      GWEN_DIALOG *dlg,
                                      uint32_t guiid)
{
  int rv;
  GtkWidget *g;

  assert(dlg);
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



GWENHYWFAR_CB int GTK2_Gui_CloseDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg)
{
  GtkWidget *g;
  int rv;

  assert(dlg);
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



GWENHYWFAR_CB int GTK2_Gui_RunDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, int untilEnd)
{
  int rv;

  assert(dlg);
  rv=GTK2_Gui_Dialog_Run(dlg, untilEnd);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return rv;
}



GWENHYWFAR_CB int GTK2_Gui_GetFileName(GWEN_GUI *gui,
                                       const char *caption,
                                       GWEN_GUI_FILENAME_TYPE fnt,
                                       uint32_t flags,
                                       const char *patterns,
                                       GWEN_BUFFER *pathBuffer,
                                       uint32_t guiid)
{
  char *folder=NULL;
  char *fileName=NULL;

  if (GWEN_Buffer_GetUsedBytes(pathBuffer)) {
    folder=strdup(GWEN_Buffer_GetStart(pathBuffer));
    fileName=strchr(folder, GWEN_DIR_SEPARATOR);
    if (fileName) {
      *fileName=0;
      fileName++;
      if (*fileName==0)
        fileName=NULL;
    }
  }

  switch (fnt) {
  case GWEN_Gui_FileNameType_OpenFileName: {
    GtkWidget *dialog;

    if (!(caption && *caption))
      caption=I18N("Open File");
    dialog=gtk_file_chooser_dialog_new(caption,
                                       NULL,
                                       GTK_FILE_CHOOSER_ACTION_OPEN,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                       NULL);
    if (folder && *folder)
      gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), folder);
    if (fileName && *fileName)
      gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), fileName);

    if (gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT) {
      char *filename;

      filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
      GWEN_Buffer_Reset(pathBuffer);
      GWEN_Buffer_AppendString(pathBuffer, filename);
      g_free(filename);
      gtk_widget_destroy(dialog);
      free(folder);
      return 0;
    }
    gtk_widget_destroy(dialog);
    free(folder);
    return GWEN_ERROR_USER_ABORTED;
  }

  case GWEN_Gui_FileNameType_SaveFileName: {
    GtkWidget *dialog;

    if (!(caption && *caption))
      caption=I18N("Save File");
    dialog=gtk_file_chooser_dialog_new(caption,
                                       NULL,
                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                       NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    if (folder && *folder)
      gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), folder);
    if (fileName && *fileName)
      gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), fileName);

    if (gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT) {
      char *filename;

      filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
      GWEN_Buffer_Reset(pathBuffer);
      GWEN_Buffer_AppendString(pathBuffer, filename);
      g_free(filename);
      gtk_widget_destroy(dialog);
      free(folder);
      return 0;
    }
    gtk_widget_destroy(dialog);
    free(folder);
    return GWEN_ERROR_USER_ABORTED;
  }

  case GWEN_Gui_FileNameType_OpenDirectory: {
    GtkWidget *dialog;

    if (!(caption && *caption))
      caption=I18N("Select Folder");
    dialog=gtk_file_chooser_dialog_new(caption,
                                       NULL,
                                       GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                       NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT) {
      char *filename;

      filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
      GWEN_Buffer_Reset(pathBuffer);
      GWEN_Buffer_AppendString(pathBuffer, filename);
      g_free(filename);
      gtk_widget_destroy(dialog);
      free(folder);
      return 0;
    }
    gtk_widget_destroy(dialog);
    free(folder);
    return GWEN_ERROR_USER_ABORTED;
  }

  default:
    break;
  }
  free(folder);

  return GWEN_ERROR_USER_ABORTED;
}





