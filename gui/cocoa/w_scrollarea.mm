/***************************************************************************
 begin       : Aug 17 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaScrollBox.h"



static GWENHYWFAR_CB
int CocoaGui_WScrollArea_SetIntProperty(GWEN_WIDGET *w,
										GWEN_DIALOG_PROPERTY prop,
										int index,
										int value,
										int doSignal) {
	
	CocoaScrollBox *scrollView;
	
	scrollView = (CocoaScrollBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_CONTENT));
	assert(scrollView);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 0;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [scrollView frame];
			frame.size.width = value;
			[scrollView setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height:{
			NSRect frame = [scrollView frame];
			frame.size.height = value;
			[scrollView setFrame:frame];
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
int CocoaGui_WScrollArea_GetIntProperty(GWEN_WIDGET *w,
										GWEN_DIALOG_PROPERTY prop,
										int index,
										int defaultValue) {
	CocoaScrollBox *scrollView;
	
	scrollView = (CocoaScrollBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_CONTENT));
	assert(scrollView);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 1;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [scrollView frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [scrollView frame].size.height;
			
		default:
			break;
	}
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WScrollArea_SetCharProperty(GWEN_WIDGET *w,
										 GWEN_DIALOG_PROPERTY prop,
										 int index,
										 const char *value,
										 int doSignal) {
	CocoaScrollBox *scrollView;
	
	scrollView = (CocoaScrollBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_CONTENT));
	assert(scrollView);
	
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* CocoaGui_WScrollArea_GetCharProperty(GWEN_WIDGET *w,
												 GWEN_DIALOG_PROPERTY prop,
												 int index,
												 const char *defaultValue) {
	CocoaScrollBox *scrollView;
	
	scrollView = (CocoaScrollBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_CONTENT));
	assert(scrollView);
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WScrollArea_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild) {
	
	
	CocoaScrollBox *scrollView;
	NSView *childView;
	uint32_t cflags;
	
	scrollView=(CocoaScrollBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(scrollView);
	
	childView=(NSView*)(GWEN_Widget_GetImplData(wChild, COCOA_DIALOG_WIDGET_REAL));
	assert(childView);
	
	cflags=GWEN_Widget_GetFlags(wChild);
	
	[scrollView setLayoutedDocumentView:childView];
	 
	return 0;
}



int CocoaGui_WScrollArea_Setup(GWEN_WIDGET *w) {
	//GtkWidget *g;
	//GtkWidget *gContent;
	CocoaScrollBox *scrollView;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	
	/* create widget */
	/*g=gtk_scrolled_window_new(NULL, NULL);
	 gContent=gtk_vbox_new(TRUE, GTK2_GUI_DIALOG_DEFAULT_BOX_SPACING);
	 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(g), gContent);*/
	
	scrollView = [[[CocoaScrollBox alloc] initWithFrame:NSMakeRect(0.0, 0.0, 100.0, 100.0)] autorelease];
	[scrollView setHasVerticalScroller:YES];
	[scrollView setHasHorizontalScroller:YES];
	[scrollView setAutohidesScrollers:NO];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) scrollView.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) scrollView.fillY = YES;
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) scrollView);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) scrollView);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WScrollArea_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WScrollArea_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WScrollArea_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WScrollArea_GetCharProperty);
	GWEN_Widget_SetAddChildGuiWidgetFn(w, CocoaGui_WScrollArea_AddChildGuiWidget);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


