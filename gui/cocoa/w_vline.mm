/***************************************************************************
 begin       : Auf 10 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaVLineView.h"


static GWENHYWFAR_CB
int CocoaGui_WVLine_SetIntProperty(GWEN_WIDGET *w,
								   GWEN_DIALOG_PROPERTY prop,
								   int index,
								   int value,
								   int doSignal) {
	CocoaVLineView *vLineView;
	
	vLineView=(CocoaVLineView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(vLineView);
	
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
int CocoaGui_WVLine_GetIntProperty(GWEN_WIDGET *w,
								   GWEN_DIALOG_PROPERTY prop,
								   int index,
								   int defaultValue) {
	CocoaVLineView *vLineView;
	
	vLineView=(CocoaVLineView*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(vLineView);
	
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			//return (gtk_widget_get_sensitive(GTK_WIDGET(g))==TRUE)?1:0;
			return 0;
			
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



int CocoaGui_WVLine_Setup(GWEN_WIDGET *w) {
	CocoaVLineView *vLineView;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	
	vLineView = [[[CocoaVLineView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 100.0, 5.0)] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) vLineView.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) vLineView.fillY = YES;
	
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) vLineView);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) vLineView);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WHLine_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WHLine_GetIntProperty);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


