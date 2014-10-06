/***************************************************************************
 begin       : Wed Feb 17 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "dlg_message_p.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/debug.h>




GWEN_INHERIT(GWEN_DIALOG, GWEN_DLGMSG)





GWEN_DIALOG *GWEN_DlgMessage_new(uint32_t flags,
                                 const char *title,
                                 const char *text,
                                 const char *b1,
                                 const char *b2,
                                 const char *b3) {
  GWEN_DIALOG *dlg;
  GWEN_DLGMSG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;
  int confirmButton;

  dlg=GWEN_Dialog_new("dlg_gwen_message");
  GWEN_NEW_OBJECT(GWEN_DLGMSG, xdlg);

  GWEN_INHERIT_SETDATA(GWEN_DIALOG, GWEN_DLGMSG, dlg, xdlg,
                       GWEN_DlgMessage_FreeData);

  GWEN_Dialog_SetSignalHandler(dlg, GWEN_DlgMessage_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
                               "gwenhywfar/dialogs/dlg_message.dlg",
                               fbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  confirmButton=GWEN_GUI_MSG_FLAGS_CONFIRM_BUTTON(flags);


  xdlg->flags=flags;
  if (title)
    xdlg->title=strdup(title);
  if (text)
    xdlg->text=strdup(text);

  if (b1 && *b1) {
    GWEN_Dialog_SetWidgetText(dlg, "button1", b1);
    if (confirmButton==1)
      GWEN_Dialog_AddWidgetFlags(dlg, "button1", GWEN_WIDGET_FLAGS_DEFAULT_WIDGET);
  }
  else
    GWEN_Dialog_RemoveWidget(dlg, "button1");


  if (b2 && *b2) {
    GWEN_Dialog_SetWidgetText(dlg, "button2", b2);
    if (confirmButton==2)
      GWEN_Dialog_AddWidgetFlags(dlg, "button2", GWEN_WIDGET_FLAGS_DEFAULT_WIDGET);
  }
  else
    GWEN_Dialog_RemoveWidget(dlg, "button2");

  if (b3 && *b3) {
    GWEN_Dialog_SetWidgetText(dlg, "button3", b3);
    if (confirmButton==3)
      GWEN_Dialog_AddWidgetFlags(dlg, "button3", GWEN_WIDGET_FLAGS_DEFAULT_WIDGET);
  }
  else
    GWEN_Dialog_RemoveWidget(dlg, "button3");

  return dlg;
}



void GWENHYWFAR_CB GWEN_DlgMessage_FreeData(void *bp, void *p) {
  GWEN_DLGMSG *xdlg;

  xdlg=(GWEN_DLGMSG*) p;

  free(xdlg->title);
  free(xdlg->text);

  GWEN_FREE_OBJECT(xdlg);
}



int GWEN_DlgMessage_GetResponse(const GWEN_DIALOG *dlg) {
  GWEN_DLGMSG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGMSG, dlg);
  assert(xdlg);

  return xdlg->response;
}



void GWEN_DlgMessage_Init(GWEN_DIALOG *dlg) {
  GWEN_DLGMSG *xdlg;
  GWEN_DB_NODE *dbParams;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGMSG, dlg);
  assert(xdlg);

  dbParams=GWEN_Dialog_GetPreferences(dlg);
  assert(dbParams);

#if 0
  /* read width */
  i=GWEN_DB_GetIntValue(dbParams, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbParams, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
#endif

  /* special stuff */
  if (xdlg->title)
    GWEN_Dialog_SetCharProperty(dlg, "", GWEN_DialogProperty_Title, 0, xdlg->title, 0);

  if (xdlg->text)
    GWEN_Dialog_SetCharProperty(dlg, "descrLabel", GWEN_DialogProperty_Title, 0, xdlg->text, 0);


  xdlg->wasInit=1;
}



void GWEN_DlgMessage_Fini(GWEN_DIALOG *dlg) {
  GWEN_DLGMSG *xdlg;
  GWEN_DB_NODE *dbParams;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGMSG, dlg);
  assert(xdlg);

  dbParams=GWEN_Dialog_GetPreferences(dlg);
  assert(dbParams);

#if 0
  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
  if (i<DIALOG_MINWIDTH)
    i=DIALOG_MINWIDTH;
  GWEN_DB_SetIntValue(dbParams,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "dialog_width",
                      i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  if (i<DIALOG_MINHEIGHT)
    i=DIALOG_MINHEIGHT;
  GWEN_DB_SetIntValue(dbParams,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "dialog_height",
                      i);
#endif
}




int GWEN_DlgMessage_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  GWEN_DLGMSG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGMSG, dlg);
  assert(xdlg);

  if (strcasecmp(sender, "button1")==0) {
    xdlg->response=1;
    return GWEN_DialogEvent_ResultAccept;
  }
  else if (strcasecmp(sender, "button2")==0) {
    xdlg->response=2;
    return GWEN_DialogEvent_ResultAccept;
  }
  else if (strcasecmp(sender, "button3")==0) {
    xdlg->response=3;
    return GWEN_DialogEvent_ResultAccept;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}




int GWENHYWFAR_CB GWEN_DlgMessage_SignalHandler(GWEN_DIALOG *dlg,
    GWEN_DIALOG_EVENTTYPE t,
    const char *sender) {
  GWEN_DLGMSG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGMSG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    GWEN_DlgMessage_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    GWEN_DlgMessage_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
    return GWEN_DlgMessage_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeValueChanged:
  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:

  case GWEN_DialogEvent_TypeClose:
    return GWEN_DialogEvent_ResultAccept;

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;

}




