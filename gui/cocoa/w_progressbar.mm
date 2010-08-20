/***************************************************************************
 begin       : August 10 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaProgressbar.h"


typedef struct W_PROGRESSBAR W_PROGRESSBAR;
struct W_PROGRESSBAR {
	int minValue;
	int maxValue;
	int currentValue;
};


GWEN_INHERIT(GWEN_WIDGET, W_PROGRESSBAR)



static GWENHYWFAR_CB
int CocoaGui_WProgressBar_SetIntProperty(GWEN_WIDGET *w,
										 GWEN_DIALOG_PROPERTY prop,
										 int index,
										 int value,
										 int doSignal) {
	CocoaProgressbar *progressbar;
	W_PROGRESSBAR *xw;
	
	assert(w);
	xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_PROGRESSBAR, w);
	assert(xw);
	
	progressbar=(CocoaProgressbar*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(progressbar);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 0;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [progressbar frame];
			frame.size.width = value;
			[progressbar setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height: {
			NSRect frame = [progressbar frame];
			frame.size.height = value;
			[progressbar setFrame:frame];
		}
			
			return 0;
			
		case GWEN_DialogProperty_Value: {
			double d;
			
			xw->currentValue=value;
			d=(double)value;
			[progressbar setDoubleValue:d];
			//NSLog(@"progressbar value = %f",d);
			return 0;
		}
			
		case GWEN_DialogProperty_MinValue: {
			double d;
			
			xw->minValue=value;
			d=(double)value;
			[progressbar setMinValue:d];
			//NSLog(@"progressbar min value = %f",d);
			return 0;
		}
			
		case GWEN_DialogProperty_MaxValue: {
			double d;
			
			xw->maxValue=value;
			d=(double)value;
			[progressbar setMaxValue:d];
			//NSLog(@"progressbar max value = %f",d);
			return 0;
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
int CocoaGui_WProgressBar_GetIntProperty(GWEN_WIDGET *w,
										 GWEN_DIALOG_PROPERTY prop,
										 int index,
										 int defaultValue) {
	CocoaProgressbar *progressbar;
	W_PROGRESSBAR *xw;
	
	assert(w);
	xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, W_PROGRESSBAR, w);
	assert(xw);
	
	progressbar=(CocoaProgressbar*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(progressbar);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 1;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [progressbar frame].size.width;
		case GWEN_DialogProperty_Height:
			return [progressbar frame].size.height;
			
		case GWEN_DialogProperty_Value:
			return xw->currentValue;
			
		case GWEN_DialogProperty_MinValue:
			return xw->minValue;
			
		case GWEN_DialogProperty_MaxValue:
			return xw->maxValue;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WProgressBar_SetCharProperty(GWEN_WIDGET *w,
										  GWEN_DIALOG_PROPERTY prop,
										  int index,
										  const char *value,
										  int doSignal) {
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* CocoaGui_WProgressBar_GetCharProperty(GWEN_WIDGET *w,
												  GWEN_DIALOG_PROPERTY prop,
												  int index,
												  const char *defaultValue) {
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static void GWENHYWFAR_CB CocoaGui_WProgressBar_FreeData(void *bp, void *p) {
	W_PROGRESSBAR *xw;
	
	xw=(W_PROGRESSBAR*) p;
	GWEN_FREE_OBJECT(xw);
}



int CocoaGui_WProgressBar_Setup(GWEN_WIDGET *w) {
	CocoaProgressbar *progressbar;
	//const char *s;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	W_PROGRESSBAR *xw;
	
	GWEN_NEW_OBJECT(W_PROGRESSBAR, xw);
	GWEN_INHERIT_SETDATA(GWEN_WIDGET, W_PROGRESSBAR, w, xw, CocoaGui_WProgressBar_FreeData);
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	//s=GWEN_Widget_GetText(w, 0);
	
	//g=gtk_progress_bar_new();
	progressbar = [[[CocoaProgressbar alloc] initWithFrame:NSMakeRect(0.0, 0.0, 100.0, 20.0)] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) progressbar.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) progressbar.fillY = YES;
	[progressbar setIndeterminate:NO];
	[progressbar setUsesThreadedAnimation:YES];
	[progressbar startAnimation:nil];
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) progressbar);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) progressbar);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WProgressBar_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WProgressBar_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WProgressBar_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WProgressBar_GetCharProperty);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


