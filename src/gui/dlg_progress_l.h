/***************************************************************************
 begin       : Tue Feb 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_DLG_PROGRESS_L_H
#define GWEN_GUI_DLG_PROGRESS_L_H


#include "progressdata_l.h"


#include <gwenhywfar/dialog.h>



GWEN_DIALOG *GWEN_DlgProgress_new(void);


GWEN_PROGRESS_DATA *GWEN_DlgProgress_GetFirstProgress(const GWEN_DIALOG *dlg);
void GWEN_DlgProgress_SetFirstProgress(GWEN_DIALOG *dlg, GWEN_PROGRESS_DATA *pd);

GWEN_PROGRESS_DATA *GWEN_DlgProgress_GetSecondProgress(const GWEN_DIALOG *dlg);
void GWEN_DlgProgress_SetSecondProgress(GWEN_DIALOG *dlg, GWEN_PROGRESS_DATA *pd);

void GWEN_DlgProgress_AddLogText(GWEN_DIALOG *dlg,
				 GWEN_LOGGER_LEVEL level,
				 const char *s);

void GWEN_DlgProgress_Advanced(GWEN_DIALOG *dlg, GWEN_PROGRESS_DATA *pd);
void GWEN_DlgProgress_TotalChanged(GWEN_DIALOG *dlg, GWEN_PROGRESS_DATA *pd);

void GWEN_DlgProgress_SetAllowClose(GWEN_DIALOG *dlg, int b);

void GWEN_DlgProgress_SetStayOpen(GWEN_DIALOG *dlg, int b);
int GWEN_DlgProgress_GetStayOpen(const GWEN_DIALOG *dlg);

void GWEN_DlgProgress_SetShowLog(GWEN_DIALOG *dlg, int b);


#endif



