/***************************************************************************
 begin       : August 16 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#import "CocoaPopUpButton.h"
#import "CocoaComboBox.h"



static GWENHYWFAR_CB
int CocoaGui_WComboBox_SetIntProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  int value,
									  int doSignal) {
	CocoaComboBox *comboBox = nil;
	CocoaPopUpButton *popUp = nil;
	
	NSControl *control = (NSControl*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(control);
	
	if ([control isKindOfClass:[CocoaComboBox class]]) comboBox = (CocoaComboBox*)control;
	else popUp = (CocoaPopUpButton*)control;
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
				[control setEnabled:(value!=0)];
			return 0;
			
		case GWEN_DialogProperty_Focus:
				[[control window] makeFirstResponder:control];
			return 0;
			
		case GWEN_DialogProperty_Value:
			if (comboBox) {
				[comboBox selectItemAtIndex:value];
			}
			else if (popUp) {
				[popUp selectItemAtIndex:value];
			}

			return 0;
			
		case GWEN_DialogProperty_ClearValues: {
			if (comboBox) [comboBox removeAllItems];
			else if (popUp) [popUp removeAllItems];
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
int CocoaGui_WComboBox_GetIntProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  int defaultValue) {
	CocoaComboBox *comboBox = nil;
	CocoaPopUpButton *popUp = nil;
	
	NSControl *control = (NSControl*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(control);
	
	if ([control isKindOfClass:[CocoaComboBox class]]) comboBox = (CocoaComboBox*)control;
	else popUp = (CocoaPopUpButton*)control;
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return ([control isEnabled])?1:0;
			
		case GWEN_DialogProperty_Focus:
			if ([control window]) {
				if ([[control window] firstResponder] == control) return 1;
			}
			return 0;
			
		case GWEN_DialogProperty_Value: {
			if (comboBox) {
				return [comboBox indexOfSelectedItem];
			}
			else if (popUp) {
				return [popUp indexOfSelectedItem];
			}
			return defaultValue;
		}
			
		case GWEN_DialogProperty_ValueCount: {
			int i = -1;
			if (comboBox) {
				i = [comboBox numberOfItems];
			}
			else if (popUp) {
				i = [popUp numberOfItems];
			}
			
			DBG_ERROR(0, "Number: %d", i);
			return i;
		}
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WComboBox_SetCharProperty(GWEN_WIDGET *w,
									   GWEN_DIALOG_PROPERTY prop,
									   int index,
									   const char *value,
									   int doSignal) {
	CocoaComboBox *comboBox = nil;
	CocoaPopUpButton *popUp = nil;
	
	NSControl *control = (NSControl*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(control);
	
	if ([control isKindOfClass:[CocoaComboBox class]]) comboBox = (CocoaComboBox*)control;
	else popUp = (CocoaPopUpButton*)control;
	
	switch(prop) {
		case GWEN_DialogProperty_Value: {
			return GWEN_ERROR_NOT_FOUND;
		}
			
		case GWEN_DialogProperty_AddValue: {
			
			NSString *stringValue = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
			if (popUp && stringValue) {
				[popUp addItemWithTitle:stringValue];
			}
			else if (comboBox && stringValue) {
				[comboBox addItemWithObjectValue:stringValue];
			}

			[stringValue release];
			return 0;
		}
			
		case GWEN_DialogProperty_ClearValues: {
			if (comboBox) [comboBox removeAllItems];
			else if (popUp) [popUp removeAllItems];
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
const char* CocoaGui_WComboBox_GetCharProperty(GWEN_WIDGET *w,
											   GWEN_DIALOG_PROPERTY prop,
											   int index,
											   const char *defaultValue) {
	CocoaComboBox *comboBox = nil;
	CocoaPopUpButton *popUp = nil;
	
	NSControl *control = (NSControl*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(control);
	
	if ([control isKindOfClass:[CocoaComboBox class]]) comboBox = (CocoaComboBox*)control;
	else popUp = (CocoaPopUpButton*)control;
	
	switch(prop) {
		case GWEN_DialogProperty_Value: {
			NSString *value = nil;
			
			if (popUp) {
				value = [popUp titleOfSelectedItem];
			}
			else if (comboBox && [[comboBox itemObjectValueAtIndex:index] isKindOfClass:[NSString class]]) {
				value = [comboBox itemObjectValueAtIndex:index];
			}
			
			if (value) return [value cStringUsingEncoding:NSUTF8StringEncoding];
			return NULL;
		}
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static void CocoaGui_WComboBox_Changed_handler(NSControl *comboBoxOrPopUp, void* data) {
	GWEN_WIDGET *w;
	int rv;
	
	DBG_ERROR(0, "Changed");
	w=(GWEN_WIDGET*)data;
	assert(w);
	rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
							  GWEN_DialogEvent_TypeActivated,
							  GWEN_Widget_GetName(w));
	if (rv==GWEN_DialogEvent_ResultAccept)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
	else if (rv==GWEN_DialogEvent_ResultReject)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}

/*static void changed_handler(GtkWidget *comboBox, gpointer data) {
	GWEN_WIDGET *w;
	int rv;
	
	DBG_ERROR(0, "Changed");
	w=data;
	assert(w);
	rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
							  GWEN_DialogEvent_TypeActivated,
							  GWEN_Widget_GetName(w));
	if (rv==GWEN_DialogEvent_ResultAccept)
		Gtk2Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
	else if (rv==GWEN_DialogEvent_ResultReject)
		Gtk2Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}*/



int CocoaGui_WComboBox_Setup(GWEN_WIDGET *w) {
	// GtkWidget *g;
	//GtkCellRenderer *cr;
	//GtkListStore *store;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	//gulong changed_handler_id;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	
	
	if (flags & GWEN_WIDGET_FLAGS_READONLY) {
		//NSPopUpButton
		CocoaPopUpButton *popUp = [[[CocoaPopUpButton alloc] initWithFrame:NSMakeRect(0.0, 0.0, 60.0, 24.0) pullsDown:NO] autorelease];
		if (flags & GWEN_WIDGET_FLAGS_FILLX) popUp.fillX = YES;
		if (flags & GWEN_WIDGET_FLAGS_FILLY) popUp.fillY = YES;
		
		GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) popUp);
		GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) popUp);
		
		gwenPopUpActionPtr ptr = CocoaGui_WComboBox_Changed_handler;
		[popUp setC_PopUpActionPtr:ptr Data:w];
	}
	else {
		//NSComboBox
		CocoaComboBox *comboBox = [[[CocoaComboBox alloc] initWithFrame:NSMakeRect(0.0, 0.0, 60.0, 24.0)] autorelease];
		if (flags & GWEN_WIDGET_FLAGS_FILLX) comboBox.fillX = YES;
		if (flags & GWEN_WIDGET_FLAGS_FILLY) comboBox.fillY = YES;
		
		GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) comboBox);
		GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) comboBox);
		
		gwenComboBoxActionPtr ptr = CocoaGui_WComboBox_Changed_handler;
		[comboBox setC_ComboBoxActionPtr:ptr Data:w];
	}
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WComboBox_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WComboBox_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WComboBox_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WComboBox_GetCharProperty);
	
#pragma mark NOCH MACHEN Action setzen
	
	/*changed_handler_id=g_signal_connect(g,
	 "changed",
	 G_CALLBACK (changed_handler),
	 w);*/
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


