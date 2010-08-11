/***************************************************************************
 begin       : August 10 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





static GWENHYWFAR_CB
int CocoaGui_WTextEdit_SetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int value,
									 int doSignal) {
	NSTextField *textField;
	
	textField=(NSTextField*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(textField);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			[textField setEnabled:(value==0)?NO:YES];
			return 0;
			
		case GWEN_DialogProperty_Focus:
			if ([textField window]) {
				[[textField window] makeFirstResponder:textField];
			}
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [textField frame];
			frame.size.width = value;
			[textField setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height:{
			NSRect frame = [textField frame];
			frame.size.height = value;
			[textField setFrame:frame];
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
int CocoaGui_WTextEdit_GetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int defaultValue) {
	NSTextField *textField;
	
	textField=(NSTextField*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(textField);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return ([textField isEnabled])?1:0;
			
			
		case GWEN_DialogProperty_Focus:
			if ([textField window]) {
				if ([[textField window] firstResponder] == textField) return 1;
			}
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [textField frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [textField frame].size.height;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WTextEdit_SetCharProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  const char *value,
									  int doSignal) {
	NSTextField *textField;
	
	textField=(NSTextField*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(textField);
	
	switch(prop) {
		case GWEN_DialogProperty_Value:{
			NSString *stringValue = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
			[textField setStringValue:stringValue];
			[stringValue release];
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
const char* CocoaGui_WTextEdit_GetCharProperty(GWEN_WIDGET *w,
											  GWEN_DIALOG_PROPERTY prop,
											  int index,
											  const char *defaultValue) {
	NSTextField *textField;
	
	textField=(NSTextField*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(textField);
	
	switch(prop) {
		case GWEN_DialogProperty_Title:
			return [[textField stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



/*static void Gtk2Gui_WTextEdit_Changed_handler(GtkTextBuffer *buffer, gpointer data) {
	GWEN_WIDGET *w;
	int rv;
	
	w=data;
	assert(w);
	rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
							  GWEN_DialogEvent_TypeValueChanged,
							  GWEN_Widget_GetName(w));
	if (rv==GWEN_DialogEvent_ResultAccept)
		Gtk2Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
	else if (rv==GWEN_DialogEvent_ResultReject)
		Gtk2Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}*/



int CocoaGui_WTextEdit_Setup(GWEN_WIDGET *w) {
	NSTextField *textField;
	const char *s;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	//gulong changed_handler_id;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	s=GWEN_Widget_GetText(w, 0);
	
	/* create widget */
	textField = [[[NSTextField alloc] initWithFrame:NSMakeRect(10.0, 10.0, 100.0, 22.0)] autorelease];
	//[textField setBordered:YES];
	[textField setEditable:YES];
	
	if (s && *s) {
		NSString *stringValue = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[textField setStringValue:stringValue];
		[stringValue release];
	}
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) textField);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) textField);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WTextEdit_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WTextEdit_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WTextEdit_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WTextEdit_GetCharProperty);
	
	
#pragma mark noch machen
	/*changed_handler_id=g_signal_connect(gtk_text_view_get_buffer(GTK_TEXT_VIEW(g)),
										"changed",
										G_CALLBACK (Gtk2Gui_WTextEdit_Changed_handler),
										w);*/
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


