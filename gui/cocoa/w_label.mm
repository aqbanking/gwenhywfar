/***************************************************************************
 begin       : August 09 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaLabel.h"


static GWENHYWFAR_CB
int CocoaGui_WLabel_SetIntProperty(GWEN_WIDGET *w,
								   GWEN_DIALOG_PROPERTY prop,
								   int index,
								   int value,
								   int doSignal) {
	CocoaLabel *labelTextField;
	
	labelTextField=(CocoaLabel*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(labelTextField);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			[labelTextField setEnabled:(value==0)?NO:YES];
			return 0;
			
		case GWEN_DialogProperty_Focus:
			if ([labelTextField window]) {
				[[labelTextField window] makeFirstResponder:labelTextField];
			}
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [labelTextField frame];
			frame.size.width = value;
			[labelTextField setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height:{
			NSRect frame = [labelTextField frame];
			frame.size.height = value;
			[labelTextField setFrame:frame];
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
int CocoaGui_WLabel_GetIntProperty(GWEN_WIDGET *w,
								   GWEN_DIALOG_PROPERTY prop,
								   int index,
								   int defaultValue) {
	CocoaLabel *labelTextField;
	
	labelTextField=(CocoaLabel*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(labelTextField);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return ([labelTextField isEnabled])?1:0;
			
		case GWEN_DialogProperty_Focus:
			if ([labelTextField window]) {
				if ([[labelTextField window] firstResponder] == labelTextField) {
					return 1;
				}
			}
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [labelTextField frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [labelTextField frame].size.height;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WLabel_SetCharProperty(GWEN_WIDGET *w,
									GWEN_DIALOG_PROPERTY prop,
									int index,
									const char *value,
									int doSignal) {
	CocoaLabel *labelTextField;
	
	labelTextField=(CocoaLabel*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(labelTextField);
	
	switch(prop) {
		case GWEN_DialogProperty_Title: {
			NSString *stringValue = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
			[labelTextField setStringValue:stringValue];
			NSLog(@"%@", stringValue);
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
const char* CocoaGui_WLabel_GetCharProperty(GWEN_WIDGET *w,
											GWEN_DIALOG_PROPERTY prop,
											int index,
											const char *defaultValue) {
	CocoaLabel *labelTextField;
	
	labelTextField=(CocoaLabel*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(labelTextField);
	
	switch(prop) {
		case GWEN_DialogProperty_Title:
			return [[labelTextField stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



int CocoaGui_WLabel_Setup(GWEN_WIDGET *w) {
	
	CocoaLabel *labelTextField;
	const char *s;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	s=GWEN_Widget_GetText(w, 0);
	
	labelTextField = [[[CocoaLabel alloc] initWithFrame:NSMakeRect(10.0, 10.0, 100.0, 24.0)] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) labelTextField.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) labelTextField.fillY = YES;
	[labelTextField setDrawsBackground:NO];
	[labelTextField setBordered:NO];
	[labelTextField setEditable:NO];
	if (s && *s) {
		NSString *stringValue = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[labelTextField setStringValue:stringValue];
		//NSLog(@"%@", stringValue);
		[stringValue release];
	}
	
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) labelTextField);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) labelTextField);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WLabel_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WLabel_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WLabel_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WLabel_GetCharProperty);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	
	
	return 0;
}


