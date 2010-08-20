/***************************************************************************
 begin       : August 16 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



/*typedef struct GTK2_GRIDLAYOUT_WIDGET GTK2_GRIDLAYOUT_WIDGET;
 struct GTK2_GRIDLAYOUT_WIDGET {
 int sortByRow;
 int allocatedColumns;
 int allocatedRows;
 
 int currentColumn;
 int currentRow;
 };*/


//GWEN_INHERIT(GWEN_WIDGET, GTK2_GRIDLAYOUT_WIDGET)

#import "CocoaGridLayout.h"


static GWENHYWFAR_CB
int CocoaGui_WGridLayout_SetIntProperty(GWEN_WIDGET *w,
										GWEN_DIALOG_PROPERTY prop,
										int index,
										int value,
										int doSignal) {
	CocoaGridLayout *gridLayout;
	
	gridLayout = (CocoaGridLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(gridLayout);
	
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
int CocoaGui_WGridLayout_GetIntProperty(GWEN_WIDGET *w,
										GWEN_DIALOG_PROPERTY prop,
										int index,
										int defaultValue) {
	CocoaGridLayout *gridLayout;
	
	gridLayout = (CocoaGridLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(gridLayout);
	
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
int CocoaGui_WGridLayout_SetCharProperty(GWEN_WIDGET *w,
										 GWEN_DIALOG_PROPERTY prop,
										 int index,
										 const char *value,
										 int doSignal) {
	CocoaGridLayout *gridLayout;
	
	gridLayout = (CocoaGridLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(gridLayout);
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char* CocoaGui_WGridLayout_GetCharProperty(GWEN_WIDGET *w,
												 GWEN_DIALOG_PROPERTY prop,
												 int index,
												 const char *defaultValue) {
	CocoaGridLayout *gridLayout;
	
	gridLayout = (CocoaGridLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(gridLayout);
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WGridLayout_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild) {
	CocoaGridLayout *gridLayout;
	NSView *childView;
	uint32_t cflags;
	
	gridLayout=(CocoaGridLayout*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(gridLayout);
	
	childView=(NSView*)(GWEN_Widget_GetImplData(wChild, COCOA_DIALOG_WIDGET_REAL));
	assert(childView);
	
	cflags=GWEN_Widget_GetFlags(wChild);
	
	[gridLayout addLayoutSubview:childView];
		
	return 0;
	
	/*  GTK2_GRIDLAYOUT_WIDGET *xw;
	 GtkWidget *g;
	 GtkWidget *gChild;
	 uint32_t cflags;
	 int x;
	 int y;
	 
	 assert(w);
	 xw=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GTK2_GRIDLAYOUT_WIDGET, w);
	 assert(xw);
	 
	 cflags=GWEN_Widget_GetFlags(wChild);
	 
	 g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK2_DIALOG_WIDGET_REAL));
	 assert(g);
	 
	 gChild=GTK_WIDGET(GWEN_Widget_GetImplData(wChild, GTK2_DIALOG_WIDGET_REAL));
	 assert(gChild);
	 
	 if (xw->sortByRow) {
	 // fill rows, enter next column if column full
	 y=(xw->currentRow)++;
	 if (y>=xw->allocatedRows) {
	 xw->currentRow=0;
	 y=(xw->currentRow)++;
	 xw->currentColumn++;
	 }
	 
	 x=xw->currentColumn;
	 if (x>=xw->allocatedColumns) {
	 xw->allocatedColumns=x+1;
	 gtk_table_resize(GTK_TABLE(g), xw->allocatedRows, xw->allocatedColumns);
	 }
	 }
	 else {
	 // fill columns, enter next row if row full
	 x=(xw->currentColumn)++;
	 if (x>=xw->allocatedColumns) {
	 xw->currentColumn=0;
	 x=(xw->currentColumn)++;
	 xw->currentRow++;
	 }
	 
	 y=xw->currentRow;
	 if (y>=xw->allocatedRows) {
	 xw->allocatedRows=y+1;
	 gtk_table_resize(GTK_TABLE(g), xw->allocatedRows, xw->allocatedColumns);
	 }
	 }
	 
	 gtk_table_attach(GTK_TABLE(g), gChild,
	 x, x+1, y, y+1,
	 (cflags & GWEN_WIDGET_FLAGS_FILLX)?(GTK_FILL|GTK_EXPAND):0,
	 (cflags & GWEN_WIDGET_FLAGS_FILLY)?(GTK_FILL|GTK_EXPAND):0,
	 GTK2_GUI_DIALOG_DEFAULT_BOX_SPACING,
	 GTK2_GUI_DIALOG_DEFAULT_BOX_SPACING);*/
	
	return 0;
}




/*static GWENHYWFAR_CB
 void Gtk2Gui_WGridLayout_FreeData(void *bp, void *p) {
 GTK2_GRIDLAYOUT_WIDGET *xw;
 
 xw=(GTK2_GRIDLAYOUT_WIDGET*) p;
 
 GWEN_FREE_OBJECT(xw);
 }*/



int CocoaGui_WGridLayout_Setup(GWEN_WIDGET *w) {
	CocoaGridLayout *gridLayout;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	//GTK2_GRIDLAYOUT_WIDGET *xw;
	int rows;
	int cols;
	
	//GWEN_NEW_OBJECT(GTK2_GRIDLAYOUT_WIDGET, xw);
	//GWEN_INHERIT_SETDATA(GWEN_WIDGET, GTK2_GRIDLAYOUT_WIDGET, w, xw, Gtk2Gui_WGridLayout_FreeData);*/
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	cols=GWEN_Widget_GetColumns(w);
	rows=GWEN_Widget_GetRows(w);
	
	
	gridLayout = [[[CocoaGridLayout alloc] initWithFrame:NSMakeRect(10.0, 10.0, 200.0, 200.0)] autorelease];
	gridLayout.columns = cols;
	gridLayout.rows = rows;
	if (flags & GWEN_WIDGET_FLAGS_FILLX) gridLayout.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) gridLayout.fillY = YES;
	
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) gridLayout);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) gridLayout);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WGridLayout_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WGridLayout_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WGridLayout_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WGridLayout_GetCharProperty);
	GWEN_Widget_SetAddChildGuiWidgetFn(w, CocoaGui_WGridLayout_AddChildGuiWidget);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


