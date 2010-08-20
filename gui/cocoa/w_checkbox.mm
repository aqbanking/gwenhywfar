/***************************************************************************
 begin       : Aug 10 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaCheckboxButton.h"


static GWENHYWFAR_CB
int CocoaGui_WCheckBox_SetIntProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  int value,
									  int doSignal) {
	CocoaCheckboxButton *checkbox;
	
	checkbox = (CocoaCheckboxButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(checkbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			[checkbox setEnabled:(value==0)?NO:YES];
			return 0;
			
		case GWEN_DialogProperty_Focus:
			if ([checkbox window]) {
				[[checkbox window] makeFirstResponder:checkbox];
			}
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [checkbox frame];
			frame.size.width = value;
			[checkbox setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height: {
			NSRect frame = [checkbox frame];
			frame.size.height = value;
			[checkbox setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Value:{
			if (value==0) [checkbox setState:NSOffState];
			else [checkbox setState:NSOnState];
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
int CocoaGui_WCheckBox_GetIntProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  int defaultValue) {
	CocoaCheckboxButton *checkbox;
	
	checkbox = (CocoaCheckboxButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(checkbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return ([checkbox isEnabled]==YES)?1:0;
			
		case GWEN_DialogProperty_Focus:
			if ([checkbox window]) {
				if ([[checkbox window] firstResponder] == checkbox) return 1;
			}
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [checkbox frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [checkbox frame].size.height;
			
		case GWEN_DialogProperty_Value:
			return ([checkbox state]==NSOnState)?1:0;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WCheckBox_SetCharProperty(GWEN_WIDGET *w,
									   GWEN_DIALOG_PROPERTY prop,
									   int index,
									   const char *value,
									   int doSignal) {
	CocoaCheckboxButton *checkbox;
	
	checkbox = (CocoaCheckboxButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(checkbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Title: {
			NSString *stringValue = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
			[checkbox setTitle:stringValue];
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
const char* CocoaGui_WCheckBox_GetCharProperty(GWEN_WIDGET *w,
											   GWEN_DIALOG_PROPERTY prop,
											   int index,
											   const char *defaultValue) {
	CocoaCheckboxButton *checkbox;
	
	checkbox = (CocoaCheckboxButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(checkbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Title:
			return [[checkbox stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static void CocoaGui_WCheckBox_Toggled_handler(NSButton *button, void* data) {
	GWEN_WIDGET *w;
	int rv;
	
	DBG_ERROR(0, "Toggled");
	w=(GWEN_WIDGET*)data;
	assert(w);
	rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
							  GWEN_DialogEvent_TypeActivated,
							  GWEN_Widget_GetName(w));
	if (rv==GWEN_DialogEvent_ResultAccept)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
	else if (rv==GWEN_DialogEvent_ResultReject)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}



int CocoaGui_WCheckBox_Setup(GWEN_WIDGET *w) {
	CocoaCheckboxButton *checkbox;
	const char *s;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	s=GWEN_Widget_GetText(w, 0);
	
	//Create Button
	checkbox = [[[CocoaCheckboxButton alloc] initWithFrame:NSMakeRect(0.0, 0.0, 60.0, 24.0)] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) checkbox.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) checkbox.fillY = YES;
	if (s && *s) {
		NSString *title = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[checkbox setTitle:title];
		[title release];
	}
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) checkbox);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) checkbox);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WCheckBox_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WCheckBox_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WCheckBox_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WCheckBox_GetCharProperty);
	
	gwenActionPtr ptr = CocoaGui_WCheckBox_Toggled_handler;
	[checkbox setC_ActionPtr:ptr Data:w];
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


