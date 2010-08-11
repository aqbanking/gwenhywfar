/***************************************************************************
 begin       : August 09 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





static GWENHYWFAR_CB
int CocoaGui_WDialog_SetIntProperty(GWEN_WIDGET *w,
									GWEN_DIALOG_PROPERTY prop,
									int index,
									int value,
									int doSignal) {
	NSWindow *window = nil;
	
	window=(NSWindow*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(window);
	
	if (window) {
		switch(prop) {
			case GWEN_DialogProperty_Enabled:
				// just ignore
				return 0;
				
			case GWEN_DialogProperty_Focus:
				[window makeKeyAndOrderFront:nil];
				return 0;
				
			case GWEN_DialogProperty_Width: {
				NSLog(@"setWidth %i", value);
				NSRect frame = [window frame];
				frame.size.width = value;
				[window setFrame:frame display:YES];
			}
				return 0;
				
			case GWEN_DialogProperty_Height: {
				NSRect frame = [window frame];
				frame.size.height = value;
				[window setFrame:frame display:YES];
			}
				return 0;
				
			default:
				break;
		}
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return GWEN_ERROR_INVALID;
}




static GWENHYWFAR_CB
int CocoaGui_WDialog_GetIntProperty(GWEN_WIDGET *w,
									GWEN_DIALOG_PROPERTY prop,
									int index,
									int defaultValue) {
	NSWindow *window = nil;
	
	window=(NSWindow*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(window);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 1;
			
		case GWEN_DialogProperty_Focus:
			return [window isKeyWindow];
			
		case GWEN_DialogProperty_Width:
			return [window frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [window frame].size.height;
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WDialog_SetCharProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 const char *value,
									 int doSignal) {
	NSWindow *window = nil;
	
	NSLog(@"CocoaGui_WDialog_SetCharProperty");
	
	window=(NSWindow*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(window);
	
	switch(prop) {
		case GWEN_DialogProperty_Title: {
			NSString *newTitle = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
			[window setTitle:newTitle];
			[newTitle release];
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
const char* CocoaGui_WDialog_GetCharProperty(GWEN_WIDGET *w,
											 GWEN_DIALOG_PROPERTY prop,
											 int index,
											 const char *defaultValue) {
	NSWindow *window = nil;
	
	window=(NSWindow*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(window);
	
	switch(prop) {
		case GWEN_DialogProperty_Title:
			return [[window title] cStringUsingEncoding:NSUTF8StringEncoding];
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WDialog_AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild) {
	
	NSLog(@"CocoaGui_WDialog_AddChildGuiWidget");
	
	NSWindow *window = nil;
	window=(NSWindow*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(window);
	
	NSView *subview = (NSView*)(GWEN_Widget_GetImplData(wChild, COCOA_DIALOG_WIDGET_REAL));
	assert(subview);
	
	NSRect bounds = [[window contentView] bounds];
	[subview setFrame:bounds];
	[subview setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
	[[window contentView] addSubview:subview];
	
	return 0;
}



int CocoaGui_WDialog_Setup(GWEN_WIDGET *w) {
	NSWindow *newWindow;
	const char *s;
	uint32_t flags;
	
	flags=GWEN_Widget_GetFlags(w);
	s=GWEN_Widget_GetText(w, 0);
	
	newWindow=[[NSWindow alloc] initWithContentRect:NSMakeRect(50.0, 50.0, 400.0, 200.0) styleMask:NSTitledWindowMask|NSClosableWindowMask|NSResizableWindowMask backing:NSBackingStoreBuffered defer:YES];
	[[newWindow contentView] setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
	//gtk_window_set_modal(GTK_WINDOW(g), TRUE);
	NSLog(@"getWindow Title");
	if (s && *s) {
		//DBG_WARN(GWEN_LOGDOMAIN, "String s = (%s)",s );
		NSString *stringValue = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		NSLog(@"Window Title = %@", stringValue);
		[newWindow setTitle:stringValue];
		[stringValue release];
	}
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) newWindow);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) newWindow);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WDialog_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WDialog_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WDialog_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WDialog_GetCharProperty);
	GWEN_Widget_SetAddChildGuiWidgetFn(w, CocoaGui_WDialog_AddChildGuiWidget);
	
	return 0;
}


