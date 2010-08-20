/***************************************************************************
 begin       : August 19 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



/*
 * Note: This is a special use case for gtk_notebook.
 * To use it as a WidgetStack we have to disable drawing od tabs
 * and borders by calling:
 * - gtk_notebook_set_show_tabs(GTK_NOTEBOOK(g), FALSE);
 * - gtk_notebook_set_show_border(GTK_NOTEBOOK(g), FALSE);
 *
 */



static GWENHYWFAR_CB
int CocoaGui_WStack_SetIntProperty(GWEN_WIDGET *w,
								   GWEN_DIALOG_PROPERTY prop,
								   int index,
								   int value,
								   int doSignal) {
	CocoaTabView *stackView;
	
	stackView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(stackView);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 0;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [stackView frame];
			frame.size.width = value;
			[stackView setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height: {
			NSRect frame = [stackView frame];
			frame.size.height = value;
			[stackView setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Value:
			if (value >= 0 && value < [stackView numberOfTabViewItems]) {
				[stackView selectTabViewItemAtIndex:value];
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
int CocoaGui_WStack_GetIntProperty(GWEN_WIDGET *w,
								   GWEN_DIALOG_PROPERTY prop,
								   int index,
								   int defaultValue) {
	CocoaTabView *stackView;
	
	stackView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(stackView);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 1;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [stackView frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [stackView frame].size.height;
			
		case GWEN_DialogProperty_Value:
			return [stackView indexOfTabViewItem:[stackView selectedTabViewItem]];
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WStack_SetCharProperty(GWEN_WIDGET *w,
									GWEN_DIALOG_PROPERTY prop,
									int index,
									const char *value,
									int doSignal) {
	CocoaTabView *stackView;
	
	stackView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(stackView);
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* CocoaGui_WStack_GetCharProperty(GWEN_WIDGET *w,
											GWEN_DIALOG_PROPERTY prop,
											int index,
											const char *defaultValue) {
	CocoaTabView *stackView;
	
	stackView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(stackView);
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WStack_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild) {
	CocoaTabView *stackView;	
	
	stackView=(CocoaTabView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(stackView);
	
	NSView *subview =(NSView*)(GWEN_Widget_GetImplData(wChild, COCOA_DIALOG_WIDGET_REAL));
	assert(subview);
	
	NSTabViewItem *item = [[NSTabViewItem alloc] init];
	
	[item setView:subview];
	
	[stackView addTabViewItem:item];
	[item release];
	
	return 0;
}



int CocoaGui_WStack_Setup(GWEN_WIDGET *w) {
	CocoaTabView *stackView;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	
	stackView = [[[CocoaTabView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 100.0, 100.0)] autorelease];
	[stackView setTabViewType:NSNoTabsNoBorder];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) stackView.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) stackView.fillY = YES;

	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) stackView);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) stackView);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WStack_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WStack_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WStack_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WStack_GetCharProperty);
	GWEN_Widget_SetAddChildGuiWidgetFn(w, CocoaGui_WStack_AddChildGuiWidget);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


