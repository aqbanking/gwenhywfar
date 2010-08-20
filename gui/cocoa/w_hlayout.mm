/***************************************************************************
 begin       : August 10 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#import "CocoaHLayout.h"



static GWENHYWFAR_CB
int CocoaGui_WHLayout_SetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int value,
									 int doSignal) {
	CocoaHLayout *hlayout;
	
	hlayout=(CocoaHLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(hlayout);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			//gtk_widget_set_sensitive(GTK_WIDGET(g), (value==0)?FALSE:TRUE);
			return 0;
			
		case GWEN_DialogProperty_Focus:
			//gtk_widget_grab_focus(GTK_WIDGET(g));
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
int CocoaGui_WHLayout_GetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int defaultValue) {
	CocoaHLayout *hlayout;
	
	hlayout=(CocoaHLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(hlayout);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			//return (gtk_widget_get_sensitive(GTK_WIDGET(g))==TRUE)?1:0;
			return 1;
			
		case GWEN_DialogProperty_Focus:
			//return (gtk_widget_has_focus(GTK_WIDGET(g))==TRUE)?1:0;
			return 0;
			
		default:
			break;
	}	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WHLayout_SetCharProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  const char *value,
									  int doSignal) {
	/*CocoaHLayout *hlayout;
	
	hlayout=(CocoaHLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(hlayout);*/
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* CocoaGui_WHLayout_GetCharProperty(GWEN_WIDGET *w,
											  GWEN_DIALOG_PROPERTY prop,
											  int index,
											  const char *defaultValue) {
	/*CocoaHLayout *hlayout;
	
	hlayout=(CocoaHLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(hlayout);*/
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WHLayout_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild) {
	CocoaHLayout *hlayout;
	NSView *childView;
	uint32_t cflags;
	
	hlayout=(CocoaHLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(hlayout);
	
	childView=(NSView*)(GWEN_Widget_GetImplData(wChild, COCOA_DIALOG_WIDGET_REAL));
	assert(childView);
	
	cflags=GWEN_Widget_GetFlags(wChild);
	
	[hlayout addLayoutSubview:childView];
	
	/*gtk_box_pack_start(GTK_BOX(g), gChild,
					   (cflags & GWEN_WIDGET_FLAGS_FILLX)?TRUE:FALSE,
					   (cflags & GWEN_WIDGET_FLAGS_FILLX)?TRUE:FALSE,
					   0);*/
	
	return 0;
}



int CocoaGui_WHLayout_Setup(GWEN_WIDGET *w) {
	CocoaHLayout *hlayout;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	
	
	hlayout = [[[CocoaHLayout alloc] initWithFrame:NSMakeRect(10.0, 10.0, 200.0, 200.0)] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) hlayout.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) hlayout.fillY = YES;
	/*#if 0
	 // using equal width here doesn't seem to be working as expected:
	 // I would expect al children be of equal width, but instead all children are
	 // equally distant to each other while still being of different width...
	 //
	 g=gtk_hbox_new((flags & GWEN_WIDGET_FLAGS_EQUAL_WIDTH)?TRUE:FALSE,
	 GTK2_GUI_DIALOG_DEFAULT_BOX_SPACING);
	 #else
	 g=gtk_hbox_new(FALSE, GTK2_GUI_DIALOG_DEFAULT_BOX_SPACING);
	 #endif*/
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) hlayout);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) hlayout);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WHLayout_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WHLayout_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WHLayout_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WHLayout_GetCharProperty);
	GWEN_Widget_SetAddChildGuiWidgetFn(w, CocoaGui_WHLayout_AddChildGuiWidget);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


