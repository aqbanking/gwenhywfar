/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GTK2_GUI_P_H
#define GTK2_GUI_P_H


#include "gtk2_gui.h"

#include <gwenhywfar/gui_be.h>



typedef struct GTK2_GUI GTK2_GUI;
struct GTK2_GUI {

  GWEN_GUI_EXEC_DIALOG_FN execDialogFn;

  GWEN_GUI_OPEN_DIALOG_FN openDialogFn;
  GWEN_GUI_CLOSE_DIALOG_FN closeDialogFn;
  GWEN_GUI_RUN_DIALOG_FN runDialogFn;
  GWEN_GUI_GET_FILENAME_FN getFileNameDialogFn;
};

static GWENHYWFAR_CB
void Gtk2_Gui_FreeData(void *bp, void *p);


static GWENHYWFAR_CB
int GTK2_Gui_ExecDialog(GWEN_GUI *gui,
			GWEN_DIALOG *dlg,
			uint32_t guiid);


static GWENHYWFAR_CB
int GTK2_Gui_OpenDialog(GWEN_GUI *gui,
			GWEN_DIALOG *dlg,
			uint32_t guiid);

static GWENHYWFAR_CB
int GTK2_Gui_CloseDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg);

static GWENHYWFAR_CB
int GTK2_Gui_RunDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, int timeout);

static GWENHYWFAR_CB
int GTK2_Gui_GetFileName(GWEN_GUI *gui,
			 const char *caption,
			 GWEN_GUI_FILENAME_TYPE fnt,
			 uint32_t flags,
			 const char *patterns,
			 GWEN_BUFFER *pathBuffer,
			 uint32_t guiid);


#endif


