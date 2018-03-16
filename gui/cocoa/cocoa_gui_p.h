/***************************************************************************
 begin       : August 03 2010
 copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef COCOA_GUI_P_H
#define COCOA_GUI_P_H

#include "cocoa_gui.h"


typedef struct COCOA_GUI COCOA_GUI;
struct COCOA_GUI {

  GWEN_GUI_EXEC_DIALOG_FN execDialogFn;

  GWEN_GUI_OPEN_DIALOG_FN openDialogFn;
  GWEN_GUI_CLOSE_DIALOG_FN closeDialogFn;
  GWEN_GUI_RUN_DIALOG_FN runDialogFn;
  GWEN_GUI_GET_FILENAME_FN getFileNameDialogFn;
};




static GWENHYWFAR_CB
void Cocoa_Gui_FreeData(void *bp, void *p);

static GWENHYWFAR_CB
int COCOA_Gui_ExecDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, uint32_t guiid);

static GWENHYWFAR_CB
int COCOA_Gui_OpenDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, uint32_t guiid);

static GWENHYWFAR_CB
int COCOA_Gui_CloseDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg);

static GWENHYWFAR_CB
int COCOA_Gui_RunDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, int timeout);

static GWENHYWFAR_CB
int COCOA_Gui_GetFileName(GWEN_GUI *gui,
                          const char *caption,
                          GWEN_GUI_FILENAME_TYPE fnt,
                          uint32_t flags,
                          const char *patterns,
                          GWEN_BUFFER *pathBuffer,
                          uint32_t guiid);


#endif
