/***************************************************************************
 begin       : August 19 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#import "CocoaSpinbox.h"


/*typedef struct W_SPINBOX W_SPINBOX;
 struct W_SPINBOX {
 GtkAdjustment *adjustment;
 };*/


GWEN_INHERIT(GWEN_WIDGET, W_SPINBOX)



static GWENHYWFAR_CB
int CocoaGui_WSpinBox_SetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int value,
									 int doSignal) {
	
	CocoaSpinbox *spinbox;
	
	spinbox=(CocoaSpinbox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(spinbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			[spinbox setEnabled:(value==0)?NO:YES];
			return 0;
			
		case GWEN_DialogProperty_Focus:
			[spinbox makeFirstResponder];
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [spinbox frame];
			frame.size.width = value;
			[spinbox setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height:{
			NSRect frame = [spinbox frame];
			frame.size.height = value;
			[spinbox setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Value:
			[spinbox setIntegerValue:value];
			return 0;
			
		case GWEN_DialogProperty_MinValue:
			[spinbox setMinValue:value];
			return 0;
			
		case GWEN_DialogProperty_MaxValue:
			[spinbox setMaxValue:value];
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
int CocoaGui_WSpinBox_GetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int defaultValue) {
	CocoaSpinbox *spinbox;
	
	spinbox=(CocoaSpinbox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(spinbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return ([spinbox isEnabled])?1:0;
			
		case GWEN_DialogProperty_Focus:
			return ([spinbox isFirstResponder])?1:0;
			
		case GWEN_DialogProperty_Width:
			return [spinbox frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [spinbox frame].size.height;
			
		case GWEN_DialogProperty_Value:
			return [spinbox integerValue];
			
		case GWEN_DialogProperty_MinValue:
			return [spinbox minValue];
			
		case GWEN_DialogProperty_MaxValue:
			return [spinbox maxValue];
			
		default:
			break;
	}
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WSpinBox_SetCharProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  const char *value,
									  int doSignal) {
	
	CocoaSpinbox *spinbox;
	
	spinbox=(CocoaSpinbox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(spinbox);
	
	
	switch(prop) {
		case GWEN_DialogProperty_Value: {
			if (value && *value) {
				NSString *stringValue = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
				[spinbox setStringValue:stringValue];
				[stringValue release];
			}
		}
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* CocoaGui_WSpinBox_GetCharProperty(GWEN_WIDGET *w,
											  GWEN_DIALOG_PROPERTY prop,
											  int index,
											  const char *defaultValue) {
	CocoaSpinbox *spinbox;
	
	spinbox=(CocoaSpinbox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(spinbox);
	
	
	switch(prop) {
		case GWEN_DialogProperty_Value:
			return [[spinbox stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}


static void CocoaGui_WSpinBox_Changed_handler(NSView *spinbox, void* data) {
	GWEN_WIDGET *w;
	int rv;
	
	DBG_ERROR(0, "ValueChanged");
	w=data;
	assert(w);
	rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
							  GWEN_DialogEvent_TypeValueChanged,
							  GWEN_Widget_GetName(w));
	if (rv==GWEN_DialogEvent_ResultAccept)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
	else if (rv==GWEN_DialogEvent_ResultReject)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}



int CocoaGui_WSpinBox_Setup(GWEN_WIDGET *w) {
	CocoaSpinbox *spinbox;
	const char *s;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	s=GWEN_Widget_GetText(w, 0);
	
	
	
	
	
	spinbox = [[[CocoaSpinbox alloc] initWithFrame:NSMakeRect(0.0, 0.0, 100.0, 22.0)] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) spinbox.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) spinbox.fillY = YES;
	
	if (s && *s) {
		NSString *stringValue = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[spinbox setStringValue:stringValue];
		[stringValue release];
	}
	else {
		[spinbox setStringValue:nil];
	}
		
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) spinbox);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) spinbox);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WSpinBox_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WSpinBox_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WSpinBox_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WSpinBox_GetCharProperty);
	
	
	gwenSpinBoxActionPtr ptr = CocoaGui_WSpinBox_Changed_handler;
	[spinbox setC_ActionPtr:ptr Data:w];
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


