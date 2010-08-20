/***************************************************************************
 begin       : Aug 10 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaRadioButton.h"


static GWENHYWFAR_CB
int CocoaGui_WRadioButton_SetIntProperty(GWEN_WIDGET *w,
										 GWEN_DIALOG_PROPERTY prop,
										 int index,
										 int value,
										 int doSignal) {
	NSButton *radioButton;
	
	radioButton = (NSButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(radioButton);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			[radioButton setEnabled:(value==0)?NO:YES];
			return 0;
			
		case GWEN_DialogProperty_Focus:
			if ([radioButton window]) {
				[[radioButton window] makeFirstResponder:radioButton];
			}
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [radioButton frame];
			frame.size.width = value;
			[radioButton setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height: {
			NSRect frame = [radioButton frame];
			frame.size.height = value;
			[radioButton setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Value:{
			if (value==0) [radioButton setState:NSOffState];
			else [radioButton setState:NSOnState];
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
int CocoaGui_WRadioButton_GetIntProperty(GWEN_WIDGET *w,
										 GWEN_DIALOG_PROPERTY prop,
										 int index,
										 int defaultValue) {
	NSButton *radioButton;
	
	radioButton = (NSButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(radioButton);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return ([radioButton isEnabled]==YES)?1:0;
			
		case GWEN_DialogProperty_Focus:
			if ([radioButton window]) {
				if ([[radioButton window] firstResponder] == radioButton) return 1;
			}
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [radioButton frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [radioButton frame].size.height;
			
		case GWEN_DialogProperty_Value:
			return ([radioButton state]==NSOnState)?1:0;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WRadioButton_SetCharProperty(GWEN_WIDGET *w,
										  GWEN_DIALOG_PROPERTY prop,
										  int index,
										  const char *value,
										  int doSignal) {
	NSButton *radioButton;
	
	radioButton = (NSButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(radioButton);
	
	switch(prop) {
		case GWEN_DialogProperty_Title: {
			NSString *stringValue = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
			[radioButton setTitle:stringValue];
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
const char* CocoaGui_WRadioButton_GetCharProperty(GWEN_WIDGET *w,
												  GWEN_DIALOG_PROPERTY prop,
												  int index,
												  const char *defaultValue) {
	NSButton *radioButton;
	
	radioButton = (NSButton*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(radioButton);
	
	switch(prop) {
		case GWEN_DialogProperty_Title:
			return [[radioButton stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static void CocoaGui_WRadioButton_Toggled_handler(NSButton *button, void* data) {
	GWEN_WIDGET *w;
	int rv;
	
	DBG_ERROR(0, "Toggled");
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



int CocoaGui_WRadioButton_Setup(GWEN_WIDGET *w) {
	CocoaRadioButton *radioButton;
	const char *s;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	GWEN_WIDGET *wT;
	int groupId;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	groupId=GWEN_Widget_GetGroupId(w);
	
	s=GWEN_Widget_GetText(w, 0);
	
	//Create Button
	radioButton = [[[CocoaRadioButton alloc] initWithFrame:NSMakeRect(0.0, 0.0, 60.0, 24.0)] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) radioButton.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) radioButton.fillY = YES;
	if (s && *s) {
		NSString *title = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[radioButton setTitle:title];
		[title release];
	}
	
	/* get root widget */
	wT=wParent;
	while(GWEN_Widget_Tree_GetParent(wT))
		wT=GWEN_Widget_Tree_GetParent(wT);
	
	// get first radio button of the same group
	while(wT) {
		if ((GWEN_Widget_GetType(wT)==GWEN_Widget_TypeRadioButton) &&
			GWEN_Widget_GetGroupId(wT)==groupId)
			break;
		wT=GWEN_Widget_Tree_GetBelow(wT);
	}
	
	if (wT && wT!=w) {
		CocoaRadioButton *sameGroupRadioButton = (CocoaRadioButton*)(GWEN_Widget_GetImplData(wT, COCOA_DIALOG_WIDGET_REAL));
		[radioButton setGroupManager:[sameGroupRadioButton getGroupManager]];
	}
	else {
		[radioButton createNewGroupManagerWithGroupID:groupId];
	}

	
	// set pointers
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) radioButton);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) radioButton);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WRadioButton_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WRadioButton_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WRadioButton_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WRadioButton_GetCharProperty);
	
	gwenActionPtr ptr = CocoaGui_WRadioButton_Toggled_handler;
	[radioButton setC_ActionPtr:ptr Data:w];
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


