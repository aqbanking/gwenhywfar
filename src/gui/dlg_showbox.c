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


#include "dlg_showbox_p.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/debug.h>



#define DIALOG_MINWIDTH  200
#define DIALOG_MINHEIGHT 50



GWEN_INHERIT(GWEN_DIALOG, GWEN_DLGSHOWBOX)





GWEN_DIALOG *GWEN_DlgShowBox_new(uint32_t flags,
				 const char *title,
				 const char *text) {
  GWEN_DIALOG *dlg;
  GWEN_DLGSHOWBOX *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("dlg_gwen_showbox");
  GWEN_NEW_OBJECT(GWEN_DLGSHOWBOX, xdlg);

  GWEN_INHERIT_SETDATA(GWEN_DIALOG, GWEN_DLGSHOWBOX, dlg, xdlg,
		       GWEN_DlgShowBox_FreeData);

  GWEN_Dialog_SetSignalHandler(dlg, GWEN_DlgShowBox_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
			       "gwenhywfar/dialogs/dlg_showbox.dlg",
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

  return dlg;
}



void GWENHYWFAR_CB GWEN_DlgShowBox_FreeData(void *bp, void *p) {
  GWEN_DLGSHOWBOX *xdlg;

  xdlg=(GWEN_DLGSHOWBOX*) p;

  free(xdlg->title);
  free(xdlg->text);

  GWEN_FREE_OBJECT(xdlg);
}



void GWEN_DlgShowBox_Init(GWEN_DIALOG *dlg) {
  GWEN_DLGSHOWBOX *xdlg;
  int i;
  GWEN_DB_NODE *dbParams;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGSHOWBOX, dlg);
  assert(xdlg);

  dbParams=GWEN_Dialog_GetPreferences(dlg);
  assert(dbParams);

  /* read width */
  i=GWEN_DB_GetIntValue(dbParams, "dialog_width", 0, -1);
  if (i<DIALOG_MINWIDTH)
    i=DIALOG_MINWIDTH;
  GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbParams, "dialog_height", 0, -1);
  if (i<DIALOG_MINHEIGHT)
    i=DIALOG_MINHEIGHT;
  GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  /* special stuff */
  if (xdlg->title)
    GWEN_Dialog_SetCharProperty(dlg, "", GWEN_DialogProperty_Title, 0, xdlg->title, 0);

  if (xdlg->text)
    GWEN_Dialog_SetCharProperty(dlg, "descrLabel", GWEN_DialogProperty_Title, 0, xdlg->text, 0);


  xdlg->wasInit=1;
}



void GWEN_DlgShowBox_Fini(GWEN_DIALOG *dlg) {
  GWEN_DLGSHOWBOX *xdlg;
  int i;
  GWEN_DB_NODE *dbParams;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGSHOWBOX, dlg);
  assert(xdlg);

  dbParams=GWEN_Dialog_GetPreferences(dlg);
  assert(dbParams);

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
}




int GWENHYWFAR_CB GWEN_DlgShowBox_SignalHandler(GWEN_DIALOG *dlg,
						GWEN_DIALOG_EVENTTYPE t,
						const char *sender) {
  GWEN_DLGSHOWBOX *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGSHOWBOX, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    GWEN_DlgShowBox_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    GWEN_DlgShowBox_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeActivated:
  case GWEN_DialogEvent_TypeValueChanged:
  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeGetImagePath:
  case GWEN_DialogEvent_TypeGetIconPath:

  case GWEN_DialogEvent_TypeClose:
    return GWEN_DialogEvent_ResultAccept;

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;

}




