/***************************************************************************
 begin       : Wed Feb 17 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_DLG_MESSAGE_P_H
#define GWEN_GUI_DLG_MESSAGE_P_H


#include "dlg_message_l.h"


struct GWEN_DLGMSG {
  int wasInit;
  uint32_t flags;
  char *title;
  char *text;

  int response;
};
typedef struct GWEN_DLGMSG GWEN_DLGMSG;




static int GWENHYWFAR_CB GWEN_DlgMessage_SignalHandler(GWEN_DIALOG *dlg,
    GWEN_DIALOG_EVENTTYPE t,
    const char *sender);

static void GWENHYWFAR_CB GWEN_DlgMessage_FreeData(void *bp, void *p);



#endif



