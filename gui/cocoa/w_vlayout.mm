/***************************************************************************
 begin       : August 10 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#import "CocoaVLayout.h"
#include "CocoaVLayout.m"



static GWENHYWFAR_CB
int CocoaGui_WVLayout_SetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int value,
									 int doSignal) {
	CocoaVLayout *vlayout;
	
	vlayout=(CocoaVLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(vlayout);
	
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
int CocoaGui_WVLayout_GetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int defaultValue) {
	CocoaVLayout *vlayout;
	
	vlayout=(CocoaVLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(vlayout);
	
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
int CocoaGui_WVLayout_SetCharProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  const char *value,
									  int doSignal) {
	/*CocoaVLayout *vlayout;
	 
	 vlayout=(CocoaVLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	 assert(vlayout);*/
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* CocoaGui_WVLayout_GetCharProperty(GWEN_WIDGET *w,
											  GWEN_DIALOG_PROPERTY prop,
											  int index,
											  const char *defaultValue) {
	/*CocoaVLayout *vlayout;
	 
	 vlayout=(CocoaVLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	 assert(vlayout);*/
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WVLayout_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild) {
	CocoaVLayout *vlayout;
	 NSView *childView;
	 uint32_t cflags;
	 
	 vlayout=(CocoaVLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	 assert(vlayout);
	 
	 childView=(NSView*)(GWEN_Widget_GetImplData(wChild, COCOA_DIALOG_WIDGET_REAL));
	 assert(childView);
	 
	 cflags=GWEN_Widget_GetFlags(wChild);
	
	
	[vlayout addLayoutSubview:childView];
	 
	//Noch richtig implementieren
	/*
	 gtk_box_pack_start(GTK_BOX(g), gChild,
	 (cflags & GWEN_WIDGET_FLAGS_FILLY)?TRUE:FALSE,
	 (cflags & GWEN_WIDGET_FLAGS_FILLY)?TRUE:FALSE,
	 0);*/
	
	return 0;
}



int CocoaGui_WVLayout_Setup(GWEN_WIDGET *w) {
	/*GtkWidget *g;
	 uint32_t flags;
	 GWEN_WIDGET *wParent;
	 
	 flags=GWEN_Widget_GetFlags(w);
	 wParent=GWEN_Widget_Tree_GetParent(w);
	 
	 g=gtk_vbox_new((flags & GWEN_WIDGET_FLAGS_EQUAL_HEIGHT)?TRUE:FALSE,
	 GTK2_GUI_DIALOG_DEFAULT_BOX_SPACING);
	 GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_REAL, (void*) g);
	 GWEN_Widget_SetImplData(w, GTK2_DIALOG_WIDGET_CONTENT, (void*) g);
	 
	 GWEN_Widget_SetSetIntPropertyFn(w, Gtk2Gui_WVLayout_SetIntProperty);
	 GWEN_Widget_SetGetIntPropertyFn(w, Gtk2Gui_WVLayout_GetIntProperty);
	 GWEN_Widget_SetSetCharPropertyFn(w, Gtk2Gui_WVLayout_SetCharProperty);
	 GWEN_Widget_SetGetCharPropertyFn(w, Gtk2Gui_WVLayout_GetCharProperty);
	 GWEN_Widget_SetAddChildGuiWidgetFn(w, Gtk2Gui_WVLayout_AddChildGuiWidget);
	 
	 if (wParent)
	 GWEN_Widget_AddChildGuiWidget(wParent, w);*/
	
	CocoaVLayout *vlayout;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	
	vlayout = [[CocoaVLayout alloc] initWithFrame:NSMakeRect(10.0, 10.0, 200.0, 200.0)];
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) vlayout);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) vlayout);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WVLayout_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WVLayout_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WVLayout_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WVLayout_GetCharProperty);
	GWEN_Widget_SetAddChildGuiWidgetFn(w, CocoaGui_WVLayout_AddChildGuiWidget);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


