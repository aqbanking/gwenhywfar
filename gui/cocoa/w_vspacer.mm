/***************************************************************************
 begin       : Aug 17 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaSpacer.h"



static GWENHYWFAR_CB
int CocoaGui_WVSpacer_SetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int value,
									 int doSignal) {
	CocoaSpacer *vSpacer;
	
	vSpacer=(CocoaSpacer*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(vSpacer);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 0;
			
		case GWEN_DialogProperty_Focus:
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
int CocoaGui_WVSpacer_GetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int defaultValue) {
	CocoaSpacer *vSpacer;
	
	vSpacer=(CocoaSpacer*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(vSpacer);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 1;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



int CocoaGui_WVSpacer_Setup(GWEN_WIDGET *w) {
	CocoaSpacer *vSpacer;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	
  	vSpacer = [[[CocoaSpacer alloc] initWithFrame:NSMakeRect(0.0, 0.0, 10.0, 10.0)] autorelease];
	GWEN_Widget_AddFlags(w, GWEN_WIDGET_FLAGS_FILLY);
	vSpacer.fillY = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLX) vSpacer.fillX = YES;
	
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) vSpacer);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) vSpacer);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WVSpacer_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WVSpacer_GetIntProperty);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


