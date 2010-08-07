/***************************************************************************
 begin       : August 03 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "cocoa_gui_p.h"

#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_GUI, COCOA_GUI)


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)



GWEN_GUI *Cocoa_Gui_new() {
  GWEN_GUI *gui;
  COCOA_GUI *xgui;

  gui=GWEN_Gui_new();
  GWEN_NEW_OBJECT(COCOA_GUI, xgui);
  GWEN_INHERIT_SETDATA(GWEN_GUI, COCOA_GUI, gui, xgui, Cocoa_Gui_FreeData);

  GWEN_Gui_AddFlags(gui, GWEN_GUI_FLAGS_DIALOGSUPPORTED);
  GWEN_Gui_UseDialogs(gui);
  xgui->execDialogFn=GWEN_Gui_SetExecDialogFn(gui, COCOA_Gui_ExecDialog);
  xgui->openDialogFn=GWEN_Gui_SetOpenDialogFn(gui, COCOA_Gui_OpenDialog);
  xgui->closeDialogFn=GWEN_Gui_SetCloseDialogFn(gui, COCOA_Gui_CloseDialog);
  xgui->runDialogFn=GWEN_Gui_SetRunDialogFn(gui, COCOA_Gui_RunDialog);
  xgui->getFileNameDialogFn=GWEN_Gui_SetGetFileNameFn(gui, COCOA_Gui_GetFileName);

  return gui;
}



void Cocoa_Gui_FreeData(void *bp, void *p) {
  COCOA_GUI *xgui;
	
  xgui=(COCOA_GUI*) p;

  GWEN_FREE_OBJECT(xgui);
}




int COCOA_Gui_ExecDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, uint32_t guiid) {
	
}



int COCOA_Gui_OpenDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, uint32_t guiid) {
  return 0;
}



int COCOA_Gui_CloseDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg) {
  return 0;
}



int COCOA_Gui_RunDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, int untilEnd) {
  return 0;
}



int COCOA_Gui_GetFileName(GWEN_GUI *gui,
			  const char *caption,
			  GWEN_GUI_FILENAME_TYPE fnt,
			  uint32_t flags,
			  const char *patterns,
			  GWEN_BUFFER *pathBuffer,
			  uint32_t guiid) {

}



