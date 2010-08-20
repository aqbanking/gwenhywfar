/***************************************************************************
 begin       : August 03 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "cocoa_gui_dialog_l.h"
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
	NSLog(@"ExecDialog");
	
	int rv;
	
	rv = COCOA_Gui_OpenDialog(gui, dlg, guiid);
	if (rv<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
		return rv;
	}
	
	rv = COCOA_Gui_RunDialog(gui, dlg, 1);
	COCOA_Gui_CloseDialog(gui, dlg);
	if (rv<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
		return rv;
	}
	
	return rv;
}



int COCOA_Gui_OpenDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, uint32_t guiid) {
	int rv;
	NSWindow *g;
	
	NSLog(@"OpenDialog");
	
	CocoaGui_Dialog_Extend(dlg);
	rv=CocoaGui_Dialog_Setup(dlg, NULL);
	if (rv<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
		CocoaGui_Dialog_Unextend(dlg);
		return rv;
	}
	
	NSLog(@"OpenDialog Setup");
	
	g=CocoaGui_Dialog_GetMainWidget(dlg);
	if (g==NULL) {
		DBG_ERROR(GWEN_LOGDOMAIN, "No main widget");
		CocoaGui_Dialog_Unextend(dlg);
		return GWEN_ERROR_INVALID;
	}
	
	NSLog(@"OpenDialog GetMainWidget");
	
	rv=GWEN_Dialog_EmitSignalToAll(dlg, GWEN_DialogEvent_TypeInit, "");
	if (rv<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "Error initializing dialog: %d", rv);
		CocoaGui_Dialog_Unextend(dlg);
		return rv;
	}
	
	NSLog(@"OpenDialog EmitSignalToAll");
	
	/* show dialog */
	[g center];
	[g makeKeyAndOrderFront:nil];
	
	
	return 0;
}



int COCOA_Gui_CloseDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg) {
	NSWindow *g;
	int rv;
	
	g=CocoaGui_Dialog_GetMainWidget(dlg);
	if (g==NULL) {
		DBG_ERROR(GWEN_LOGDOMAIN, "No main widget");
		CocoaGui_Dialog_Unextend(dlg);
		return GWEN_ERROR_INVALID;
	}
	
	/* hide dialog */
	[g close];
	
	
	/* send fini signal to dialog */
	rv=GWEN_Dialog_EmitSignalToAll(dlg, GWEN_DialogEvent_TypeFini, "");
	if (rv<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "Error deinitializing dialog: %d", rv);
		CocoaGui_Dialog_Unextend(dlg);
		return rv;
	}
	
	CocoaGui_Dialog_Unextend(dlg);
	return 0;
}



int COCOA_Gui_RunDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, int untilEnd) {
	int rv;
	
	rv=Cocoa_Gui_Dialog_Run(dlg, untilEnd);
	if (rv<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
		return rv;
	}
	return rv;
}



int COCOA_Gui_GetFileName(GWEN_GUI *gui,
						  const char *caption,
						  GWEN_GUI_FILENAME_TYPE fnt,
						  uint32_t flags,
						  const char *patterns,
						  GWEN_BUFFER *pathBuffer,
						  uint32_t guiid) {
	char *folder=NULL;
	char *fileName=NULL;
	
	if (GWEN_Buffer_GetUsedBytes(pathBuffer)) {
		folder=strdup(GWEN_Buffer_GetStart(pathBuffer));
		fileName=strchr(folder, GWEN_DIR_SEPARATOR);
		if (fileName) {
			*fileName=0;
			fileName++;
			if (*fileName==0)
				fileName=NULL;
		}
	}
	
	switch(fnt) {
		case GWEN_Gui_FileNameType_OpenFileName:
		case GWEN_Gui_FileNameType_OpenDirectory: {
			
			NSString *title = nil;
			if (caption && *caption)
				title = [[[NSString alloc] initWithCString:caption encoding:NSUTF8StringEncoding] autorelease];
			
			NSOpenPanel *openPanel = [NSOpenPanel openPanel];
			if (openPanel) {
				if (title) [openPanel setTitle:title];
				[openPanel setAllowsMultipleSelection:NO];
				[openPanel setCanChooseFiles:(fnt == GWEN_Gui_FileNameType_OpenFileName)];
				[openPanel setCanChooseDirectories:(fnt == GWEN_Gui_FileNameType_OpenDirectory)];
				
				NSString *pathToFolder = nil;
				
				if (fileName && *fileName)
					pathToFolder = [[[NSString alloc] initWithCString:fileName encoding:NSUTF8StringEncoding] autorelease];
				
				if (pathToFolder)
					[openPanel setDirectory:pathToFolder];
				
				NSInteger result = [openPanel runModal];
				if (result == NSFileHandlingPanelOKButton) {
					NSArray *urls = [openPanel URLs];
					if (urls && [urls count] > 0) {
						NSString *path = [[(NSURL*)[urls objectAtIndex:0] path] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
						if (path && [path length]>0) {
							const char *filename = [path cStringUsingEncoding:NSUTF8StringEncoding];
							GWEN_Buffer_Reset(pathBuffer);
							GWEN_Buffer_AppendString(pathBuffer, filename);
							return 0;
						}
					}
				}
			}
			
			return GWEN_ERROR_USER_ABORTED;
		}
			
		case GWEN_Gui_FileNameType_SaveFileName: {
			
			NSString *title = nil;
			if (caption && *caption)
				title = [[[NSString alloc] initWithCString:caption encoding:NSUTF8StringEncoding] autorelease];
			
			NSSavePanel *savePanel = [NSSavePanel savePanel];
			if (savePanel) {
				if (title) [savePanel setTitle:title];
				[savePanel setCanCreateDirectories:YES];
				
				NSString *pathToFolder = nil;
				
				if (fileName && *fileName)
					pathToFolder = [[[NSString alloc] initWithCString:fileName encoding:NSUTF8StringEncoding] autorelease];
				
				if (pathToFolder)
					[savePanel setDirectory:pathToFolder];
				
				NSInteger result = [savePanel runModal];
				if (result == NSFileHandlingPanelOKButton) {
					NSURL *url = [savePanel URL];
					if (url) {
						NSString *path = [[url path] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
						if (path && [path length]>0) {
							const char *filename = [path cStringUsingEncoding:NSUTF8StringEncoding];
							GWEN_Buffer_Reset(pathBuffer);
							GWEN_Buffer_AppendString(pathBuffer, filename);
							return 0;
						}
					}
				}
			}
			
			return GWEN_ERROR_USER_ABORTED;
		}
			
		default:
			break;
	}
	free(folder);
	
	return GWEN_ERROR_USER_ABORTED;
}



