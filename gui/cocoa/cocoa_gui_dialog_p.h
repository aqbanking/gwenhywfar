/***************************************************************************
    begin       : August 09 2010
    copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef COCOA_GUI_DIALOG_P_H
#define COCOA_GUI_DIALOG_P_H


#include "cocoa_gui_dialog_l.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

typedef struct COCOA_GUI_DIALOG COCOA_GUI_DIALOG;
struct COCOA_GUI_DIALOG {
  GWEN_DIALOG_SETINTPROPERTY_FN setIntPropertyFn;
  GWEN_DIALOG_GETINTPROPERTY_FN getIntPropertyFn;
  GWEN_DIALOG_SETCHARPROPERTY_FN setCharPropertyFn;
  GWEN_DIALOG_GETCHARPROPERTY_FN getCharPropertyFn;

  NSWindow *mainWindow;

  /*int response;
  GMainLoop *loop;
  int destroyed;

  gulong unmap_handler;
  gulong destroy_handler;
  gulong delete_handler;*/

};

static void GWENHYWFAR_CB CocoaGui_Dialog_FreeData(void *bp, void *p);


static int CocoaGui_Dialog_SetIntProperty(GWEN_DIALOG *dlg,
					 GWEN_WIDGET *w,
					 GWEN_DIALOG_PROPERTY prop,
					 int index,
					 int value,
					 int doSignal);


static int CocoaGui_Dialog_GetIntProperty(GWEN_DIALOG *dlg,
					 GWEN_WIDGET *w,
					 GWEN_DIALOG_PROPERTY prop,
					 int index,
					 int defaultValue);

static int CocoaGui_Dialog_SetCharProperty(GWEN_DIALOG *dlg,
					  GWEN_WIDGET *w,
					  GWEN_DIALOG_PROPERTY prop,
					  int index,
					  const char *value,
					  int doSignal);

static const char *CocoaGui_Dialog_GetCharProperty(GWEN_DIALOG *dlg,
						  GWEN_WIDGET *w,
						  GWEN_DIALOG_PROPERTY prop,
						  int index,
						  const char *defaultValue);


static int CocoaGui_Dialog_SetupTree(GWEN_WIDGET *w);


#endif


