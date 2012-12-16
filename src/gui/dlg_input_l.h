/***************************************************************************
 begin       : Wed Feb 17 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_DLG_INPUT_L_H
#define GWEN_GUI_DLG_INPUT_L_H


#include <gwenhywfar/dialog.h>



GWEN_DIALOG *GWEN_DlgInput_new(uint32_t flags,
			       const char *title,
			       const char *text,
			       int minLen,
			       int maxLen);


int GWEN_DlgInput_CopyInput(GWEN_DIALOG *dlg, char *buffer, int size);

int GWEN_DlgInput_GetFlagAllowStore(GWEN_DIALOG *dlg);




#endif



