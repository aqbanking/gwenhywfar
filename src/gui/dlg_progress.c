/***************************************************************************
 begin       : Tue Feb 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "dlg_progress_p.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



#define DIALOG_MINWIDTH        520
#define DIALOG_MINHEIGHT       400
#define DIALOG_MINHEIGHT_NOLOG 100



GWEN_INHERIT(GWEN_DIALOG, GWEN_DLGPROGRESS)





GWEN_DIALOG *GWEN_DlgProgress_new(void) {
  GWEN_DIALOG *dlg;
  GWEN_DLGPROGRESS *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  dlg=GWEN_Dialog_new("dlg_gwen_progress");
  GWEN_NEW_OBJECT(GWEN_DLGPROGRESS, xdlg);

  GWEN_INHERIT_SETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg, xdlg,
		       GWEN_DlgProgress_FreeData);

  GWEN_Dialog_SetSignalHandler(dlg, GWEN_DlgProgress_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(GWEN_PM_LIBNAME, GWEN_PM_SYSDATADIR,
			       "gwenhywfar/dialogs/dlg_progress.dlg",
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

  xdlg->logBufferTxt=GWEN_Buffer_new(0, 256, 0, 1);
  xdlg->logBufferHtml=GWEN_Buffer_new(0, 256, 0, 1);

  return dlg;
}



void GWENHYWFAR_CB GWEN_DlgProgress_FreeData(void *bp, void *p) {
  GWEN_DLGPROGRESS *xdlg;

  xdlg=(GWEN_DLGPROGRESS*) p;

  GWEN_Buffer_free(xdlg->logBufferHtml);
  GWEN_Buffer_free(xdlg->logBufferTxt);

  GWEN_FREE_OBJECT(xdlg);
}



void GWEN_DlgProgress_SetAllowClose(GWEN_DIALOG *dlg, int b) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  xdlg->allowClose=b;
  if (xdlg->wasInit) {
    GWEN_Dialog_SetIntProperty(dlg, "abortButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "closeButton", GWEN_DialogProperty_Enabled, 0, 1, 0);
  }
}



void GWEN_DlgProgress_SetStayOpen(GWEN_DIALOG *dlg, int b) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  xdlg->stayOpen=b;
}



int GWEN_DlgProgress_GetStayOpen(const GWEN_DIALOG *dlg) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  return xdlg->stayOpen;
}



void GWEN_DlgProgress_SetShowLog(GWEN_DIALOG *dlg, int b) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  if (xdlg->showLog!=b) {
    xdlg->showLog=b;
    if (xdlg->wasInit) {

      GWEN_Dialog_SetIntProperty(dlg, "logGroup", GWEN_DialogProperty_Visibility, 0, b, 0);
      if (b) {
	int i;

        i=xdlg->withLogWidth;
	if (i<DIALOG_MINWIDTH)
	  i=DIALOG_MINWIDTH;
	GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

	i=xdlg->withLogHeight;
	if (i<DIALOG_MINHEIGHT)
	  i=DIALOG_MINHEIGHT;
	GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
      }
    }
  }
}



GWEN_PROGRESS_DATA *GWEN_DlgProgress_GetFirstProgress(const GWEN_DIALOG *dlg) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  return xdlg->firstProgress;
}



void GWEN_DlgProgress_SetFirstProgress(GWEN_DIALOG *dlg, GWEN_PROGRESS_DATA *pd) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  xdlg->firstProgress=pd;

  if (xdlg->wasInit) {
    if (xdlg->firstProgress) {
      const char *s;

      s=GWEN_ProgressData_GetTitle(xdlg->firstProgress);
      if (s && *s)
	GWEN_Dialog_SetCharProperty(dlg, "", GWEN_DialogProperty_Title, 0, s, 0);

      s=GWEN_ProgressData_GetText(xdlg->firstProgress);
      if (s && *s)
	GWEN_Dialog_SetCharProperty(dlg, "descrLabel", GWEN_DialogProperty_Title, 0, s, 0);

      GWEN_Dialog_SetIntProperty(dlg, "allProgress", GWEN_DialogProperty_Enabled, 0, 1, 0);
      GWEN_Dialog_SetIntProperty(dlg, "allProgress", GWEN_DialogProperty_MaxValue, 0,
				 GWEN_ProgressData_GetTotal(xdlg->firstProgress), 0);
      GWEN_Dialog_SetIntProperty(dlg, "allProgress", GWEN_DialogProperty_Value, 0,
				 GWEN_ProgressData_GetCurrent(xdlg->firstProgress), 0);
    }
    else {
      /* let it show 100 % */
      GWEN_Dialog_SetIntProperty(dlg, "allProgress", GWEN_DialogProperty_MaxValue, 0, 100, 0);
      GWEN_Dialog_SetIntProperty(dlg, "allProgress", GWEN_DialogProperty_Value, 0, 100, 0);
    }
  }
}



GWEN_PROGRESS_DATA *GWEN_DlgProgress_GetSecondProgress(const GWEN_DIALOG *dlg) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  return xdlg->secondProgress;
}



void GWEN_DlgProgress_SetSecondProgress(GWEN_DIALOG *dlg, GWEN_PROGRESS_DATA *pd) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  xdlg->secondProgress=pd;

  if (xdlg->wasInit) {
    if (xdlg->secondProgress) {
      GWEN_Dialog_SetIntProperty(dlg, "currentProgress", GWEN_DialogProperty_Enabled, 0, 1, 0);
      GWEN_Dialog_SetIntProperty(dlg, "currentProgress", GWEN_DialogProperty_MaxValue, 0,
				 GWEN_ProgressData_GetTotal(xdlg->secondProgress), 0);
      GWEN_Dialog_SetIntProperty(dlg, "currentProgress", GWEN_DialogProperty_Value, 0,
				 GWEN_ProgressData_GetCurrent(xdlg->secondProgress), 0);
    }
    else {
      GWEN_Dialog_SetIntProperty(dlg, "currentProgress", GWEN_DialogProperty_Value, 0, 0, 0);
      GWEN_Dialog_SetIntProperty(dlg, "currentProgress", GWEN_DialogProperty_Enabled, 0, 0, 0);
    }
  }
}



void GWEN_DlgProgress_AddLogText(GWEN_DIALOG *dlg,
				 GWEN_LOGGER_LEVEL level,
				 const char *s) {
  GWEN_DLGPROGRESS *xdlg;
  GWEN_TIME *ti;
  int rv;
  GWEN_BUFFER *tbuf;
  const char *col;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  ti=GWEN_CurrentTime();

  /* setup text string */
  if (GWEN_Buffer_GetUsedBytes(xdlg->logBufferTxt))
    GWEN_Buffer_AppendString(xdlg->logBufferTxt, "\n");
  if (ti)
    rv=GWEN_Time_toString(ti, "hh:mm:ss", xdlg->logBufferTxt);
  else
    rv=GWEN_ERROR_GENERIC;
  if (rv<0)
    GWEN_Buffer_AppendString(xdlg->logBufferTxt, "??:??:??");
  GWEN_Buffer_AppendString(xdlg->logBufferTxt, " ");
  if (s)
    GWEN_Buffer_AppendString(xdlg->logBufferTxt, s);

  /* setup HTML string */
  GWEN_Buffer_AppendString(xdlg->logBufferHtml, "<tr><td>");
  if (ti)
    rv=GWEN_Time_toString(ti, "hh:mm:ss", xdlg->logBufferHtml);
  else
    rv=GWEN_ERROR_GENERIC;
  if (rv<0)
    GWEN_Buffer_AppendString(xdlg->logBufferHtml, "??:??:??");
  GWEN_Buffer_AppendString(xdlg->logBufferHtml, "</td><td>");

  if (level<=GWEN_LoggerLevel_Error)
    col="red";
  else if (level==GWEN_LoggerLevel_Warning)
    col="blue";
  else if (level==GWEN_LoggerLevel_Info)
    col="green";
  else
    col=NULL;
  if (col) {
    GWEN_Buffer_AppendString(xdlg->logBufferHtml, "<font color=\"");
    GWEN_Buffer_AppendString(xdlg->logBufferHtml, col);
    GWEN_Buffer_AppendString(xdlg->logBufferHtml,"\">");
  }
  if (s)
    GWEN_Text_EscapeXmlToBuffer(s, xdlg->logBufferHtml);
  if (col)
    GWEN_Buffer_AppendString(xdlg->logBufferHtml, "</font>");
  GWEN_Buffer_AppendString(xdlg->logBufferHtml, "</td></tr>");

  /* assemble full string, containing HTML and text log */
  tbuf=GWEN_Buffer_new(0,
		       GWEN_Buffer_GetUsedBytes(xdlg->logBufferHtml)+
		       GWEN_Buffer_GetUsedBytes(xdlg->logBufferTxt)+256,
		       0,
		       1);

  GWEN_Buffer_AppendString(tbuf, "<html><table>");
  GWEN_Buffer_AppendString(tbuf, GWEN_Buffer_GetStart(xdlg->logBufferHtml));
  GWEN_Buffer_AppendString(tbuf, "</table></html>");
  GWEN_Buffer_AppendString(tbuf, GWEN_Buffer_GetStart(xdlg->logBufferTxt));

  GWEN_Dialog_SetCharProperty(dlg, "logText", GWEN_DialogProperty_Value, 0,
			      GWEN_Buffer_GetStart(tbuf), 0);
  GWEN_Buffer_free(tbuf);
}



void GWEN_DlgProgress_Advanced(GWEN_DIALOG *dlg, GWEN_PROGRESS_DATA *pd) {
  GWEN_DLGPROGRESS *xdlg;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  if (pd==xdlg->firstProgress)
    s="allProgress";
  else if (pd==xdlg->secondProgress)
    s="currentProgress";
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Progress %08x is neither primary nor secondary",
	      GWEN_ProgressData_GetId(pd));
    return;
  }

  if (xdlg->wasInit) {
    GWEN_Dialog_SetIntProperty(dlg, s, GWEN_DialogProperty_Value, 0,
			       GWEN_ProgressData_GetCurrent(pd), 0);
  }
}



void GWEN_DlgProgress_Init(GWEN_DIALOG *dlg) {
  GWEN_DLGPROGRESS *xdlg;
  int i;
  GWEN_DB_NODE *dbParams;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  dbParams=GWEN_Dialog_GetPreferences(dlg);
  assert(dbParams);

  xdlg->withLogWidth=GWEN_DB_GetIntValue(dbParams, "dialog_width", 0, -1);
  xdlg->withLogHeight=GWEN_DB_GetIntValue(dbParams, "dialog_height", 0, -1);

  if (xdlg->showLog) {
    int i;
  
    i=xdlg->withLogWidth;
    if (i>=DIALOG_MINWIDTH)
      GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);
  
    i=xdlg->withLogHeight;
    if (i>=DIALOG_MINHEIGHT)
      GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

    GWEN_Dialog_SetIntProperty(dlg, "logGroup", GWEN_DialogProperty_Visibility, 0, 1, 0);
  }
  else {
    /* read width */
    i=GWEN_DB_GetIntValue(dbParams, "dialog_width_nolog", 0, -1);
    if (i>=DIALOG_MINWIDTH)
      GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);
  
    /* read height */
    i=GWEN_DB_GetIntValue(dbParams, "dialog_height_nolog", 0, -1);
    if (i>=DIALOG_MINHEIGHT_NOLOG)
      GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);
    GWEN_Dialog_SetIntProperty(dlg, "logGroup", GWEN_DialogProperty_Visibility, 0, 0, 0);
  }


  if (xdlg->firstProgress) {
    const char *s;

    s=GWEN_ProgressData_GetTitle(xdlg->firstProgress);
    if (s && *s)
      GWEN_Dialog_SetCharProperty(dlg, "", GWEN_DialogProperty_Title, 0, s, 0);

    s=GWEN_ProgressData_GetText(xdlg->firstProgress);
    if (s && *s)
      GWEN_Dialog_SetCharProperty(dlg, "descrLabel", GWEN_DialogProperty_Title, 0, s, 0);

    GWEN_Dialog_SetIntProperty(dlg, "allProgress", GWEN_DialogProperty_MaxValue, 0,
			       GWEN_ProgressData_GetTotal(xdlg->firstProgress), 0);
    GWEN_Dialog_SetIntProperty(dlg, "allProgress", GWEN_DialogProperty_Value, 0,
			       GWEN_ProgressData_GetCurrent(xdlg->firstProgress), 0);
  }

  if (xdlg->secondProgress) {
    GWEN_Dialog_SetIntProperty(dlg, "currentProgress", GWEN_DialogProperty_MaxValue, 0,
			       GWEN_ProgressData_GetTotal(xdlg->secondProgress), 0);
    GWEN_Dialog_SetIntProperty(dlg, "currentProgress", GWEN_DialogProperty_Value, 0,
			       GWEN_ProgressData_GetCurrent(xdlg->secondProgress), 0);
  }

  GWEN_Dialog_SetIntProperty(dlg, "abortButton", GWEN_DialogProperty_Enabled, 0, 1, 0);
  GWEN_Dialog_SetIntProperty(dlg, "closeButton", GWEN_DialogProperty_Enabled, 0, 0, 0);

  xdlg->wasInit=1;
}



void GWEN_DlgProgress_Fini(GWEN_DIALOG *dlg) {
  GWEN_DLGPROGRESS *xdlg;
  int i;
  GWEN_DB_NODE *dbParams;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  dbParams=GWEN_Dialog_GetPreferences(dlg);
  assert(dbParams);

  /* store dialog width */
  if (xdlg->showLog) {
    i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
    GWEN_DB_SetIntValue(dbParams,
			GWEN_DB_FLAGS_OVERWRITE_VARS,
			"dialog_width",
			i);

    /* store dialog height */
    i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
    GWEN_DB_SetIntValue(dbParams,
			GWEN_DB_FLAGS_OVERWRITE_VARS,
			"dialog_height",
			i);
  }
  else {
    i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
    GWEN_DB_SetIntValue(dbParams,
			GWEN_DB_FLAGS_OVERWRITE_VARS,
			"dialog_width_nolog",
			i);

    /* store dialog height */
    i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
    GWEN_DB_SetIntValue(dbParams,
			GWEN_DB_FLAGS_OVERWRITE_VARS,
			"dialog_height_nolog",
			i);
  }
}




int GWEN_DlgProgress_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  if (strcasecmp(sender, "closeButton")==0) {
    return GWEN_DialogEvent_ResultAccept;
  }
  else if (strcasecmp(sender, "abortButton")==0) {
    if (xdlg->firstProgress)
      GWEN_ProgressData_SetAborted(xdlg->firstProgress, 1);
    if (xdlg->secondProgress)
      GWEN_ProgressData_SetAborted(xdlg->secondProgress, 1);

    xdlg->stayOpen=1;
    GWEN_Dialog_SetIntProperty(dlg, "abortButton", GWEN_DialogProperty_Enabled, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "closeButton", GWEN_DialogProperty_Enabled, 0, 1, 0);

    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;
}




int GWENHYWFAR_CB GWEN_DlgProgress_SignalHandler(GWEN_DIALOG *dlg,
						 GWEN_DIALOG_EVENTTYPE t,
						 const char *sender) {
  GWEN_DLGPROGRESS *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, GWEN_DLGPROGRESS, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    GWEN_DlgProgress_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    GWEN_DlgProgress_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    break;

  case GWEN_DialogEvent_TypeActivated:
    return GWEN_DlgProgress_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:

  case GWEN_DialogEvent_TypeClose:
    if (xdlg->allowClose==0)
      return GWEN_DialogEvent_ResultReject;
    else
      return GWEN_DialogEvent_ResultAccept;

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;
  }

  return GWEN_DialogEvent_ResultNotHandled;

}






