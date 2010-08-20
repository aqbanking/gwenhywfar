/***************************************************************************
    begin       : August 09 2010
    copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef COCOA_GUI_DIALOG_L_H
#define COCOA_GUI_DIALOG_L_H


//#include <gwen-gui-gtk2/gtk2_gui.h>


#include <gwenhywfar/dialog_be.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#import "CocoaWindow.h"

//#define GTK2_GUI_DIALOG_DEFAULT_BOX_SPACING 3


/*typedef struct {
  GWEN_DIALOG *dialog;
  GtkWindow *window;
  int response;
  GMainLoop *loop;
  int destroyed;
} RunInfo;*/



void CocoaGui_Dialog_Extend(GWEN_DIALOG *dlg);
void CocoaGui_Dialog_Unextend(GWEN_DIALOG *dlg);

int CocoaGui_Dialog_Setup(GWEN_DIALOG *dlg, NSWindow *parentWindow);


CocoaWindow *CocoaGui_Dialog_GetMainWidget(const GWEN_DIALOG *dlg);


int Cocoa_Gui_Dialog_Run(GWEN_DIALOG *dlg, int timeout);
void CocoaGui_Dialog_Leave(GWEN_DIALOG *dlg, int result);

#endif


