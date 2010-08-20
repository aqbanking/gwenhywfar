/***************************************************************************
 begin       : Aug 18 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaTextBrowser.h"



static GWENHYWFAR_CB
int CocoaGui_WTextBrowser_SetIntProperty(GWEN_WIDGET *w,
										 GWEN_DIALOG_PROPERTY prop,
										 int index,
										 int value,
										 int doSignal) {
	
	CocoaTextBrowser *textBrowser;
	textBrowser = (CocoaTextBrowser*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_CONTENT));
	assert(textBrowser);
	
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 0;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Width: {
			NSRect frame = [textBrowser frame];
			frame.size.width = value;
			[textBrowser setFrame:frame];
		}
			return 0;
			
		case GWEN_DialogProperty_Height: {
			NSRect frame = [textBrowser frame];
			frame.size.height = value;
			[textBrowser setFrame:frame];
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
int CocoaGui_WTextBrowser_GetIntProperty(GWEN_WIDGET *w,
										 GWEN_DIALOG_PROPERTY prop,
										 int index,
										 int defaultValue) {
	CocoaTextBrowser *textBrowser;
	textBrowser = (CocoaTextBrowser*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_CONTENT));
	assert(textBrowser);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return 1;
			
		case GWEN_DialogProperty_Focus:
			return 0;
			
		case GWEN_DialogProperty_Width:
			return [textBrowser frame].size.width;
			
		case GWEN_DialogProperty_Height:
			return [textBrowser frame].size.height;
						
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



static GWENHYWFAR_CB
int CocoaGui_WTextBrowser_SetCharProperty(GWEN_WIDGET *w,
										  GWEN_DIALOG_PROPERTY prop,
										  int index,
										  const char *value,
										  int doSignal) {
	CocoaTextBrowser *textBrowser;
	textBrowser = (CocoaTextBrowser*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_CONTENT));
	assert(textBrowser);
	
	switch(prop) {
		case GWEN_DialogProperty_Value: {
			if (value && *value) {
				GWEN_STRINGLIST *sl;
				
				//take the first media path as base url
				sl=GWEN_Dialog_GetMediaPaths(GWEN_Widget_GetDialog(w));
				const char * basePath = GWEN_StringListEntry_Data(GWEN_StringList_FirstEntry(sl));
				NSString *basePathString = nil;
				if (basePath && *basePath) {
					basePathString = [[NSString alloc] initWithCString:basePath encoding:NSUTF8StringEncoding];
				}
				
				NSString *stringValue = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
				[textBrowser setStringValue:stringValue BasePath:basePathString];
				[stringValue release];
				[basePathString release];
			}
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
const char* CocoaGui_WTextBrowser_GetCharProperty(GWEN_WIDGET *w,
												  GWEN_DIALOG_PROPERTY prop,
												  int index,
												  const char *defaultValue) {
	CocoaTextBrowser *textBrowser;
	textBrowser = (CocoaTextBrowser*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_CONTENT));
	assert(textBrowser);
	
	switch(prop) {
		case GWEN_DialogProperty_Value: {
			NSString* stringValue = [textBrowser stringValue];
			if (stringValue && [stringValue length] > 0)
				return [stringValue cStringUsingEncoding:NSUTF8StringEncoding];
			return defaultValue;
		}
			
		default:
			break;
	}
	
	DBG_WARN(GWEN_LOGDOMAIN,
			 "Function is not appropriate for this type of widget (%s)",
			 GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
	return defaultValue;
}



int CocoaGui_WTextBrowser_Setup(GWEN_WIDGET *w) {
	CocoaTextBrowser *textBrowser;
	const char *s;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	s=GWEN_Widget_GetText(w, 0);
	
	
	textBrowser = [[[CocoaTextBrowser alloc] initWithFrame:NSMakeRect(0.0, 0.0, 30.0, 30.0) frameName:nil groupName:nil] autorelease];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) textBrowser.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) textBrowser.fillY = YES;
	
	if (s && *s) {
		NSString *stringValue = [[NSString alloc] initWithCString:s encoding:NSUTF8StringEncoding];
		[textBrowser setStringValue:stringValue];
		[stringValue release];
	}
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) textBrowser);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) textBrowser);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WTextBrowser_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WTextBrowser_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WTextBrowser_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WTextBrowser_GetCharProperty);
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


