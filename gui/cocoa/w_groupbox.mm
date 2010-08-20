/***************************************************************************
 begin       : August 16 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaWindowContentView.h"
#import "CocoaGroupBox.h"


static GWENHYWFAR_CB
int CocoaGui_WGroupBox_SetIntProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  int value,
									  int doSignal) {
	CocoaGroupBox *box;
	
	box = (CocoaGroupBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(box);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 0;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [box frame];
			frame.size.width = value;
			[box setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height: {
			NSRect frame = [box frame];
			frame.size.height = value;
			[box setFrame:frame];
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
int CocoaGui_WGroupBox_GetIntProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  int defaultValue) {
	CocoaGroupBox *box;
	
	box = (CocoaGroupBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(box);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			//return (gtk_widget_get_sensitive(GTK_WIDGET(g))==TRUE)?1:0;
			return 1;
			
		case GWEN_DialogProperty_Focus:
			//return (gtk_widget_has_focus(GTK_WIDGET(g))==TRUE)?1:0;
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [box frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [box frame].size.height;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WGroupBox_SetCharProperty(GWEN_WIDGET *w,
									   GWEN_DIALOG_PROPERTY prop,
									   int index,
									   const char *value,
									   int doSignal) {
	CocoaGroupBox *box;
	
	box = (CocoaGroupBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(box);
	
	switch(prop) {
		case GWEN_DialogProperty_Title: {
			NSString *stringValue = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
			[box setTitle:stringValue];
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
const char* CocoaGui_WGroupBox_GetCharProperty(GWEN_WIDGET *w,
											   GWEN_DIALOG_PROPERTY prop,
											   int index,
											   const char *defaultValue) {
	CocoaGroupBox *box;
	
	box = (CocoaGroupBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(box);
	
	switch(prop) {
		case GWEN_DialogProperty_Title:
			return [[box title] cStringUsingEncoding:NSUTF8StringEncoding];
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WGroupBox_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild) {
	CocoaGroupBox *box;
	//GtkWidget *gChild;
	//uint32_t cflags;
	
	box = (CocoaGroupBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(box);
	
	NSView *subview = (NSView*)(GWEN_Widget_GetImplData(wChild, COCOA_DIALOG_WIDGET_REAL));
	assert(subview);
	
	[(CocoaWindowContentView*)[box contentView] addLayoutSubview:subview];
	
	//cflags=GWEN_Widget_GetFlags(wChild);
	
	/*gtk_box_pack_start(GTK_BOX(g), gChild,
					   (cflags & GWEN_WIDGET_FLAGS_FILLY)?TRUE:FALSE,
					   (cflags & GWEN_WIDGET_FLAGS_FILLY)?TRUE:FALSE,
					   0);*/
	
	return 0;
}



int CocoaGui_WGroupBox_Setup(GWEN_WIDGET *w) {
	CocoaGroupBox *box;
	const char *s;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	s=GWEN_Widget_GetText(w, 0);
	
	
	box = [[[CocoaGroupBox alloc] initWithFrame:NSMakeRect(10.0, 10.0, 100.0, 22.0)] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) box.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) box.fillY = YES;
	
	CocoaWindowContentView *newContentView = [[CocoaWindowContentView alloc] initWithFrame:[[box contentView] frame]];
	[newContentView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
	[box setContentView:newContentView];
	[newContentView release];
	
	if (s && *s) {
		NSString *title = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[box setTitle:title];
		[title release];
	}
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) box);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) box);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WGroupBox_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WGroupBox_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WGroupBox_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WGroupBox_GetCharProperty);
	GWEN_Widget_SetAddChildGuiWidgetFn(w, CocoaGui_WGroupBox_AddChildGuiWidget);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


