/***************************************************************************
 begin       : Tue Feb 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_DLG_PROGRESS_P_H
#define GWEN_GUI_DLG_PROGRESS_P_H


#include "dlg_progress_l.h"


struct GWEN_DLGPROGRESS {
  GWEN_PROGRESS_DATA *firstProgress;
  GWEN_PROGRESS_DATA *secondProgress;

  int wasInit;
  int allowClose;
  int stayOpen;
  int showLog;

  GWEN_BUFFER *logBufferTxt;
  GWEN_BUFFER *logBufferHtml;
};
typedef struct GWEN_DLGPROGRESS GWEN_DLGPROGRESS;




static int GWENHYWFAR_CB GWEN_DlgProgress_SignalHandler(GWEN_DIALOG *dlg,
							GWEN_DIALOG_EVENTTYPE t,
							const char *sender);

static void GWENHYWFAR_CB GWEN_DlgProgress_FreeData(void *bp, void *p);



#endif



