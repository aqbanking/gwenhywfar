/***************************************************************************
 begin       : August 10 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaGWENButton.h"
#include "CocoaGWENButton.mm"


static GWENHYWFAR_CB
int CocoaGui_WPushButton_SetIntProperty(GWEN_WIDGET *w,
										GWEN_DIALOG_PROPERTY prop,
										int index,
										int value,
										int doSignal) {
	
	NSButton *button;
	
	button=(NSButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(button);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			[button setEnabled:(value==0)?NO:YES];
			return 0;
			
		case GWEN_DialogProperty_Focus:
			if ([button window]) {
				[[button window] makeFirstResponder:button];
			}
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [button frame];
			frame.size.width = value;
			[button setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height: {
			NSRect frame = [button frame];
			frame.size.height = value;
			[button setFrame:frame];
		}
			return 0;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}




static GWENHYWFAR_CB
int CocoaGui_WPushButton_GetIntProperty(GWEN_WIDGET *w,
										GWEN_DIALOG_PROPERTY prop,
										int index,
										int defaultValue) {
	NSButton *button;
	
	button=(NSButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(button);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return ([button isEnabled]==YES)?1:0;
			
		case GWEN_DialogProperty_Focus:
			if ([button window]) {
				if ([[button window] firstResponder] == button) return 1;
			}
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [button frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [button frame].size.height;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WPushButton_SetCharProperty(GWEN_WIDGET *w,
										 GWEN_DIALOG_PROPERTY prop,
										 int index,
										 const char *value,
										 int doSignal) {
	
	NSButton *button;
	
	button=(NSButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(button);
	
	switch(prop) {
		case GWEN_DialogProperty_Title: {
			NSString *stringValue = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
			[button setTitle:stringValue];
			[stringValue release];
		}
			return 0;
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* CocoaGui_WPushButton_GetCharProperty(GWEN_WIDGET *w,
												 GWEN_DIALOG_PROPERTY prop,
												 int index,
												 const char *defaultValue) {
	NSButton *button;
	
	button=(NSButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(button);
	
	switch(prop) {
		case GWEN_DialogProperty_Title:
			return [[button stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static void CocoaGui_WPushButton_Clicked_handler(NSButton *button, void* data) {
	GWEN_WIDGET *w;
	int rv;
	
	DBG_ERROR(0, "Clicked");
	w=data;
	assert(w);
	rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
							  GWEN_DialogEvent_TypeActivated,
							  GWEN_Widget_GetName(w));
	if (rv==GWEN_DialogEvent_ResultAccept)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
	else if (rv==GWEN_DialogEvent_ResultReject)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}



int CocoaGui_WPushButton_Setup(GWEN_WIDGET *w) {
	
	CocoaGWENButton *button;
	const char *s;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	s=GWEN_Widget_GetText(w, 0);
	
	
	//Create Button
	button = [[[CocoaGWENButton alloc] initWithFrame:NSMakeRect(0.0, 0.0, 60.0, 24.0)] autorelease];
	[button setBezelStyle:NSRoundedBezelStyle];
	if (s && *s) {
		NSString *title = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[button setTitle:title];
		[title release];
	}
	
	s=GWEN_Widget_GetIconFileName(w);
	if (s && *s) {
		GWEN_STRINGLIST *sl;
		
		sl=GWEN_Dialog_GetMediaPaths(GWEN_Widget_GetDialog(w));
		if (sl) {
			int rv;
			GWEN_BUFFER *tbuf;
			
			tbuf=GWEN_Buffer_new(0, 256, 0, 1);
			rv=GWEN_Directory_FindFileInPaths(sl, s, tbuf);
			if (rv<0) {
				DBG_ERROR(GWEN_LOGDOMAIN, "Image file [%s] not found (%d)", s, rv);
				// ignore result here, instead create GtkImage with "broken mage" later
			}
			else {
				NSString *pathToIconFile = [[NSString alloc] initWithCString:GWEN_Buffer_GetStart(tbuf) encoding:NSUTF8StringEncoding];
				if (pathToIconFile) {
					NSImage *icon = [[NSImage alloc] initWithContentsOfFile:pathToIconFile];
					[pathToIconFile release];
					if (icon) {
						[button setBezelStyle:NSRegularSquareBezelStyle];
						[button setImage:icon];
						[button setImagePosition:NSImageLeft];
						[icon release];
					}
				}
			}
			GWEN_Buffer_free(tbuf);
		}
	}
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) button);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) button);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WPushButton_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WPushButton_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WPushButton_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WPushButton_GetCharProperty);
	
	gwenActionPtr ptr = CocoaGui_WPushButton_Clicked_handler;
	[button setC_ActionPtr:ptr Data:w];
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


