/***************************************************************************
 begin       : August 16 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaTabView.h"


static GWENHYWFAR_CB
int CocoaGui_WTabBook_SetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int value,
									 int doSignal) {
	CocoaTabView *tabView;
	
	tabView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(tabView);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 0;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Value:
			if (value >= 0 && value < [tabView numberOfTabViewItems]) {
				[tabView selectTabViewItemAtIndex:value];
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
int CocoaGui_WTabBook_GetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int defaultValue) {
	CocoaTabView *tabView;
	
	tabView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(tabView);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 1;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Value:
			return [tabView indexOfTabViewItem:[tabView selectedTabViewItem]];
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WTabBook_SetCharProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  const char *value,
									  int doSignal) {
	CocoaTabView *tabView;
	
	tabView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(tabView);
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* CocoaGui_WTabBook_GetCharProperty(GWEN_WIDGET *w,
											  GWEN_DIALOG_PROPERTY prop,
											  int index,
											  const char *defaultValue) {
	CocoaTabView *tabView;
	
	tabView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(tabView);
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WTabBook_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild) {
	CocoaTabView *tabView;
	const char *s;
	
	
	tabView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(tabView);
	
	NSView *subview =(NSView*)(GWEN_Widget_GetImplData(wChild, COCOA_DIALOG_WIDGET_REAL));
	assert(subview);
	
	s=GWEN_Widget_GetText(wChild, 0);
	
	
	NSTabViewItem *item = [[NSTabViewItem alloc] init];

	if (s && *s) {
		NSString *title = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[item setLabel:title];
		[title release];
	}
	
	[item setView:subview];
	
	[tabView addTabViewItem:item];
	[item release];
	
	
	return 0;
}



int CocoaGui_WTabBook_Setup(GWEN_WIDGET *w) {
	CocoaTabView *tabView;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	
	tabView=[[[CocoaTabView alloc] initWithFrame:NSMakeRect(10.0, 10.0, 100.0, 22.0)] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) tabView.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) tabView.fillY = YES;
		
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) tabView);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) tabView);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WTabBook_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WTabBook_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WTabBook_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WTabBook_GetCharProperty);
	GWEN_Widget_SetAddChildGuiWidgetFn(w, CocoaGui_WTabBook_AddChildGuiWidget);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


