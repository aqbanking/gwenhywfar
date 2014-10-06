/***************************************************************************
 begin       : Wed Feb 17 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_DLG_SHOWBOX_P_H
#define GWEN_GUI_DLG_SHOWBOX_P_H


#include "dlg_showbox_l.h"


struct GWEN_DLGSHOWBOX {
  int wasInit;
  uint32_t flags;
  char *title;
  char *text;
};
typedef struct GWEN_DLGSHOWBOX GWEN_DLGSHOWBOX;




static int GWENHYWFAR_CB GWEN_DlgShowBox_SignalHandler(GWEN_DIALOG *dlg,
    GWEN_DIALOG_EVENTTYPE t,
    const char *sender);

static void GWENHYWFAR_CB GWEN_DlgShowBox_FreeData(void *bp, void *p);



#endif



