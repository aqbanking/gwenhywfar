/***************************************************************************
 begin       : Wed Feb 17 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_DLG_INPUT_P_H
#define GWEN_GUI_DLG_INPUT_P_H


#include "dlg_input_l.h"


struct GWEN_DLGINPUT {
  int wasInit;
  uint32_t flags;
  char *title;
  char *text;
  int minLen;
  int maxLen;

  char *response;
  int flagAllowStore;
};
typedef struct GWEN_DLGINPUT GWEN_DLGINPUT;




static int GWENHYWFAR_CB GWEN_DlgInput_SignalHandler(GWEN_DIALOG *dlg,
    GWEN_DIALOG_EVENTTYPE t,
    const char *sender);

static void GWENHYWFAR_CB GWEN_DlgInput_FreeData(void *bp, void *p);



#endif



