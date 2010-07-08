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


#include "dlg_input_p.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/debug.h>




GWEN_INHERIT(GWEN_DIALOG, GWEN_DLGINPUT)





GWEN_DIALOG *GWEN_DlgInput_new(uint32_t flags,
			       const char *title,
			       const char *text,
			       int minLen,
			       int maxLen) {
  GWEN_DIALOG *dlg;
  GWEN_DLGINPUT *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  if (flags & GWEN_GUI_INPUT_FLAGS_CONFIRM)
    dlg=GWEN_Dialog_new("dlg_gwen_input1");
  else
    dlg=GWEN_Dialog_new("dlg_gwen_input2");
  GWEN_NEW_OBJECT(GWEN_DLGINPUT, xdlg);

  GWEN_INHERIT_SETDATA(GWEN_DIALOG, GWEN_DLGINPUT, dlg, xdlg,
		       GWEN_DlgInput_FreeData);

  GWEN_Dialog_SetSignalHandler(dlg, GWEN_DlgInput_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
			       "gwenhywfar/dialogs/dlg_input.dlg",
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

  xdlg->flags=flags;
  if (title)
    xdlg->title=strdup(title);
  if (text)
    xdlg->text=strdup(text);
  xdlg->minLen=minLen;
  xdlg->maxLen=maxLen;

  if (!(flags & GWEN_GUI_INPUT_FLAGS_SHOW)) {
    GWEN_Dialog_AddWidgetFlags(dlg, "input1", GWEN_WIDGET_FLAGS_PASSWORD);
    GWEN_Dialog_AddWidgetFlags(dlg, "input2", GWEN_WIDGET_FLAGS_PASSWORD);
  }

  if (maxLen>32) {
    GWEN_Dialog_SetWidgetColumns(dlg, "input1", 64);
    GWEN_Dialog_SetWidgetColumns(dlg, "input2", 64);
  }
  else {
    GWEN_Dialog_SetWidgetColumns(dlg, "input1", 32);
    GWEN_Dialog_SetWidgetColumns(dlg, "input2", 32);
  }

  if (!(flags & GWEN_GUI_INPUT_FLAGS_CONFIRM)) {
    GWEN_Dialog_RemoveWidget(dlg, "input2");
    GWEN_Dialog_RemoveWidget(dlg, "label2");
  }

  return dlg;
}



void GWENHYWFAR_CB GWEN_DlgInput_FreeData(void *bp, void *p) {
  GWEN_DLGINPUT *xdlg;

  xdlg=(GWEN_DLGINPUT*) p;

  if (xdlg->response) {
    memset(xdlg->response, 0, strlen(xdlg->response));
    xdlg->response=NULL;
  }
  free(xdlg->title);
  free(xdlg->text);

  GWEN_FREE_OBJECT(xdlg);
}



int GWEN_DlgInput_CheckInput(GWEN_DIALOG *dlg) {
  GWEN_DLGINPUT *xdlg;
  const char *s1;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGINPUT, dlg);
  assert(xdlg);

  s1=GWEN_Dialog_GetCharProperty(dlg, "input1", GWEN_DialogProperty_Value, 0, NULL);
  if (xdlg->flags & GWEN_GUI_INPUT_FLAGS_CONFIRM) {
    const char *s2;

    s2=GWEN_Dialog_GetCharProperty(dlg, "input2", GWEN_DialogProperty_Value, 0, NULL);

    /* check for equality */
    if (!s1 || !s2 || strcasecmp(s1, s2)!=0)
      return -1;
  }

  if (!s1)
    return -1;

  if (xdlg->minLen>=0) {
    if (strlen(s1)<xdlg->minLen)
      return -1;
  }

  return 0;
}



void GWEN_DlgInput_Init(GWEN_DIALOG *dlg) {
  GWEN_DLGINPUT *xdlg;
  GWEN_DB_NODE *dbParams;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGINPUT, dlg);
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


  GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "abortButton", GWEN_DialogProperty_Enabled, 0, 1, 0);

  GWEN_Dialog_SetIntProperty(dlg, "input1", GWEN_DialogProperty_Focus, 0, 1, 0);


  xdlg->wasInit=1;
}



void GWEN_DlgInput_Fini(GWEN_DIALOG *dlg) {
  GWEN_DLGINPUT *xdlg;
  GWEN_DB_NODE *dbParams;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGINPUT, dlg);
  assert(xdlg);

  dbParams=GWEN_Dialog_GetPreferences(dlg);
  assert(dbParams);

  if (xdlg->response) {
    memset(xdlg->response, 0, strlen(xdlg->response));
    xdlg->response=NULL;
  }

  if (GWEN_DlgInput_CheckInput(dlg)==0) {
    const char *s;
    s=GWEN_Dialog_GetCharProperty(dlg, "input1", GWEN_DialogProperty_Value, 0, NULL);
    if (s)
      xdlg->response=strdup(s);
  }

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




int GWEN_DlgInput_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  GWEN_DLGINPUT *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGINPUT, dlg);
  assert(xdlg);

  DBG_ERROR(0, "Activated: %s", sender);
  if (strcasecmp(sender, "okButton")==0) {
    return GWEN_DialogEvent_ResultAccept;
  }
  else if (strcasecmp(sender, "abortButton")==0) {
    return GWEN_DialogEvent_ResultReject;
  }
  else if (strcasecmp(sender, "input1")==0 ||
	   strcasecmp(sender, "input2")==0) {
    if (GWEN_DlgInput_CheckInput(dlg)==0)
      return GWEN_DialogEvent_ResultAccept;
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWEN_DlgInput_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender) {
  GWEN_DLGINPUT *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGINPUT, dlg);
  assert(xdlg);

  if (strcasecmp(sender, "input1")==0 ||
      strcasecmp(sender, "input2")==0) {
    if (GWEN_DlgInput_CheckInput(dlg))
      /* disable okButton */
      GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
    else
      /* enable okButton */
      GWEN_Dialog_SetIntProperty(dlg, "okButton", GWEN_DialogProperty_Enabled, 0, 1, 0);
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}



int GWENHYWFAR_CB GWEN_DlgInput_SignalHandler(GWEN_DIALOG *dlg,
					      GWEN_DIALOG_EVENTTYPE t,
					      const char *sender) {
  GWEN_DLGINPUT *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGINPUT, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    GWEN_DlgInput_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    GWEN_DlgInput_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return GWEN_DlgInput_HandleValueChanged(dlg, sender);
    break;

  case GWEN_DialogEvent_TypeActivated:
    return GWEN_DlgInput_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:

  case GWEN_DialogEvent_TypeClose:
    return GWEN_DialogEvent_ResultAccept;

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;

}



int GWEN_DlgInput_CopyInput(GWEN_DIALOG *dlg, char *buffer, int size) {
  GWEN_DLGINPUT *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGINPUT, dlg);
  assert(xdlg);

  if (xdlg->response) {
    int l;

    l=strlen(xdlg->response);
    if ((l+1)>size) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }
    /* buffer ok, copy */
    memmove(buffer, xdlg->response, l+1);
    return 0;
  }
  return GWEN_ERROR_NO_DATA;
}




