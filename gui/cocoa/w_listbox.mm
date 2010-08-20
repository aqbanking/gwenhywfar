/***************************************************************************
 begin       : Aug 17 2010
 copyright   : (C) 2010 by Samuel Strupp
 
 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaScrollBox.h"
#import "CocoaListBox.h"


#define W_LISTBOX_MAX_TYPES 256



static GWENHYWFAR_CB
int CocoaGui_WListBox_SetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int value,
									 int doSignal) {
	CocoaListBox *listbox;
	CocoaScrollBox *scrollView;
	
	scrollView = (CocoaScrollBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(scrollView);
	
	listbox = [scrollView documentView];
	assert(listbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			[listbox setEnabled:(value==0)?NO:YES];
			return 0;
			
		case GWEN_DialogProperty_Focus: {
			if ([listbox window]) {
				[[listbox window] makeFirstResponder:listbox];
			}
		}
			return 0;
			
		case GWEN_DialogProperty_SelectionMode: {
			switch(value) {
				case GWEN_Dialog_SelectionMode_None:
					[listbox setAllowsEmptySelection:YES];
					[listbox setAllowsMultipleSelection:NO];
					[listbox deselectAll:listbox];
					return 0;
				case GWEN_Dialog_SelectionMode_Single:
					[listbox setAllowsEmptySelection:NO];
					[listbox setAllowsMultipleSelection:NO];
					return 0;
				case GWEN_Dialog_SelectionMode_Multi:
					[listbox setAllowsEmptySelection:NO];
					[listbox setAllowsMultipleSelection:YES];
					return 0;
			}
			DBG_ERROR(GWEN_LOGDOMAIN, "Unknown SelectionMode %d", value);
			return GWEN_ERROR_INVALID;
		}
			
		case GWEN_DialogProperty_ColumnWidth: {
			
			if ([listbox setColumnWidthTo:value forColumn:index]) return 0;
			return GWEN_ERROR_INVALID;
		}
			
		case GWEN_DialogProperty_SortDirection: {
			
			if ([listbox setSortOrderTo:(value == GWEN_DialogSortDirection_Up) ForColumnWithIndex:index]) return 0;
			return GWEN_ERROR_INVALID;
		}
			
		case GWEN_DialogProperty_Sort:
			// NOOP, we use auto-sorting for now (TODO: figure out how to manually sort)
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
int CocoaGui_WListBox_GetIntProperty(GWEN_WIDGET *w,
									 GWEN_DIALOG_PROPERTY prop,
									 int index,
									 int defaultValue) {
	CocoaListBox *listbox;
	CocoaScrollBox *scrollView;
	
	scrollView = (CocoaScrollBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(scrollView);
	
	listbox = [scrollView documentView];
	assert(listbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Enabled:
			return ([listbox isEnabled])?1:0;
			
		case GWEN_DialogProperty_Focus:
			if ([listbox window]) {
				if ([[listbox window] firstResponder] == listbox)
					return 1;
			}
			return 0;
			
		case GWEN_DialogProperty_Value: {
			return [listbox selectedRow];
			return -1;
		}
			
		case GWEN_DialogProperty_ColumnWidth: {
			return [listbox widthOfColumn:index];
		}
			
		case GWEN_DialogProperty_SortDirection: {
			
			NSInteger sortOrder = [listbox sortOrderForColumnAtIndex:index];
			switch (sortOrder) {
				case 1:
					return GWEN_DialogSortDirection_Up;
				case 0:
					return GWEN_DialogSortDirection_Down;
				default:
					break;
			}
			return GWEN_DialogSortDirection_None;
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
int CocoaGui_WListBox_SetCharProperty(GWEN_WIDGET *w,
									  GWEN_DIALOG_PROPERTY prop,
									  int index,
									  const char *value,
									  int doSignal) {
	CocoaListBox *listbox;
	CocoaScrollBox *scrollView;
	
	scrollView = (CocoaScrollBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(scrollView);
	
	listbox = [scrollView documentView];
	assert(listbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Title: {
			
			if (value && *value) {
				NSString *titleString = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
				[listbox setTitelsAndCreateColumns:titleString];
				[titleString release];
			}
			return 0;
		}
			
		case GWEN_DialogProperty_ClearValues: {
			[listbox clearDataRows];
			return 0;
		}
			
		case GWEN_DialogProperty_AddValue: {
			
			if (value && *value) {
				NSString *dataRowString = [[NSString alloc] initWithCString:value encoding:NSUTF8StringEncoding];
				[listbox addDataRowString:dataRowString];
				[dataRowString release];
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
const char* CocoaGui_WListBox_GetCharProperty(GWEN_WIDGET *w,
											  GWEN_DIALOG_PROPERTY prop,
											  int index,
											  const char *defaultValue) {
	CocoaListBox *listbox;
	CocoaScrollBox *scrollView;
	
	scrollView = (CocoaScrollBox*)(GWEN_Widget_GetImplData(w, COCOA_DIALOG_WIDGET_REAL));
	assert(scrollView);
	
	listbox = [scrollView documentView];
	assert(listbox);
	
	switch(prop) {
		case GWEN_DialogProperty_Title: {
			NSString *titlesString = [listbox titlesString];
			if (titlesString) {
				return [titlesString cStringUsingEncoding:NSUTF8StringEncoding];
			}
			return defaultValue;
		}
			
		case GWEN_DialogProperty_Value: {
			NSString *dataRowString = [listbox dataRowStringForRow:index];
			if (dataRowString) {
				return [dataRowString cStringUsingEncoding:NSUTF8StringEncoding];
			}
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



static void CocoaGui_WListBox_Changed_handler(NSTableView *tableView, void* data) {
	GWEN_WIDGET *w;
	int rv;
	
	DBG_ERROR(0, "ListBox Changed");
	w=data;
	assert(w);
	rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
							  GWEN_DialogEvent_TypeActivated,
							  GWEN_Widget_GetName(w));
	if (rv==GWEN_DialogEvent_ResultAccept)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
	else if (rv==GWEN_DialogEvent_ResultReject)
		CocoaGui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}



int CocoaGui_WListBox_Setup(GWEN_WIDGET *w) {
	CocoaListBox *listBox;
	uint32_t flags;
	GWEN_WIDGET *wParent;
	//gulong changed_handler_id;
	
	flags=GWEN_Widget_GetFlags(w);
	wParent=GWEN_Widget_Tree_GetParent(w);
	
	CocoaScrollBox *scrollView = [[[CocoaScrollBox alloc] initWithFrame:NSMakeRect(0.0, 0.0, 100.0, 100.0)] autorelease];
	[scrollView setHasVerticalScroller:YES];
	[scrollView setHasHorizontalScroller:YES];
	[scrollView setAutohidesScrollers:YES];
	if (flags & GWEN_WIDGET_FLAGS_FILLX) scrollView.fillX = YES;
	if (flags & GWEN_WIDGET_FLAGS_FILLY) scrollView.fillY = YES;
	
	listBox = [[[CocoaListBox alloc] initWithFrame:NSMakeRect(0.0, 0.0, 100.0, 100.0)] autorelease];
	[listBox setFocusRingType:NSFocusRingTypeNone];
	[scrollView setLayoutedDocumentView:listBox];
	
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_REAL, (void*) scrollView);
	GWEN_Widget_SetImplData(w, COCOA_DIALOG_WIDGET_CONTENT, (void*) scrollView);
	
	GWEN_Widget_SetSetIntPropertyFn(w, CocoaGui_WListBox_SetIntProperty);
	GWEN_Widget_SetGetIntPropertyFn(w, CocoaGui_WListBox_GetIntProperty);
	GWEN_Widget_SetSetCharPropertyFn(w, CocoaGui_WListBox_SetCharProperty);
	GWEN_Widget_SetGetCharPropertyFn(w, CocoaGui_WListBox_GetCharProperty);
	
	
#pragma mark NOCH MACHEN
	/*changed_handler_id=g_signal_connect(g,
	 "cursor-changed",
	 G_CALLBACK (Gtk2Gui_WListBox_CursorChanged_handler),
	 w);*/
	
	gwenListBoxActionPtr ptr = CocoaGui_WListBox_Changed_handler;
	[listBox setC_ActionPtr:ptr Data:w];
	
	if (wParent)
		GWEN_Widget_AddChildGuiWidget(wParent, w);
	
	return 0;
}


