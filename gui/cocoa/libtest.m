/***************************************************************************
    begin       : August 03 2010
    copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "cocoa_gui_dialog.mm"
#include "cocoa_gui.mm"
//#include "../testdialogs/dlg_test.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/dialog.h>
#include <gwenhywfar/debug.h>


#include <unistd.h>

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

@interface AppDelegate : NSObject {
	
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;

@end

@implementation AppDelegate

-(id) init {
	self = [super init];
	if (self) {
		
	}
	return self;
}

-(void) dealloc {
	[super dealloc];
}

- (void)windowWillClose:(NSNotification *)notification {
	[NSApp stop:self];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	
	NSLog(@"Test");
	
	NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(100.0, 100.0, 200.0, 200.0) styleMask:NSTitledWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];
	[window setDelegate:self];
	[window makeKeyAndOrderFront:nil];
	
	test1();
}

@end


int test1() {
	//fprintf(stderr, "Test\n");
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSLog(@"Start");
	
	GWEN_GUI *gui;
	int rv;
	GWEN_DIALOG *dlg;
	
	rv=GWEN_Init();
	if (rv) {
		DBG_ERROR_ERR(0, rv);
		return 2;
	}
	
	NSLog(@"GWEN Gui initialized");
	
	GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);
	
	//gtk_set_locale ();
	//gtk_init (&argc, &argv);
	
	// create GUI
	gui=Cocoa_Gui_new();
	GWEN_Gui_SetGui(gui);
	
	NSLog(@"Cocoa Gui set");
	
	dlg=Dlg_Test1_new();
	if (dlg==NULL) {
		fprintf(stderr, "Could not create dialog.\n");
		return 2;
	}
	
	NSLog(@"Test new");
	
	rv=GWEN_Gui_ExecDialog(dlg, 0);
	fprintf(stderr, "Result: %d\n", rv);
	
	
	/* GWEN_GUI *gui;
	 int rv;
	 GWEN_DIALOG *dlg;
	 
	 rv=GWEN_Init();
	 if (rv) {
	 DBG_ERROR_ERR(0, rv);
	 return 2;
	 }
	 
	 GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);
	 
	 gtk_set_locale ();
	 gtk_init (&argc, &argv);
	 
	 // create GUI
	 gui=Gtk2_Gui_new();
	 GWEN_Gui_SetGui(gui);
	 
	 dlg=Dlg_Test1_new();
	 if (dlg==NULL) {
	 fprintf(stderr, "Could not create dialog.\n");
	 return 2;
	 }
	 
	 rv=GWEN_Gui_ExecDialog(dlg, 0);
	 fprintf(stderr, "Result: %d\n", rv);*/
	
	[pool release];
	
	return 0;
}





/*int test2(int argc, char **argv) {
  int rv;
  uint32_t id1;
  uint32_t id2;
  uint64_t i1;
  uint64_t i2;
  GWEN_GUI *gui;

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  gui=Gtk2_Gui_new();
  GWEN_Gui_SetGui(gui);

  id1=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_LOG |
			     GWEN_GUI_PROGRESS_SHOW_ABORT |
			     GWEN_GUI_PROGRESS_KEEP_OPEN,
			     "Progress-Title",
                             "<html>"
                             "<p><b>This</b> is an example <i>text</i>..</p>"
                             "<p>As you can see <font color=red>colors</font> can "
                             "be used.</p>"
                             "</html>",
                             10,
                             0);
  for (i1=1; i1<=10; i1++) {
    char numbuf[128];

    snprintf(numbuf, sizeof(numbuf)-1, "Step %d\n", (int)i1);
    GWEN_Gui_ProgressLog(id1, GWEN_LoggerLevel_Notice, numbuf);
    id2=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_LOG |
                               GWEN_GUI_PROGRESS_DELAY |
			       GWEN_GUI_PROGRESS_SHOW_ABORT,
			       "2nd progress",
			       "Starting 2nd progress...",
			       10,
			       id1);
    for (i2=1; i2<=10; i2++) {
      sleep(1);
      fprintf(stderr, "Advancing %d/%d\n", (int)i1, (int)i2);
      rv=GWEN_Gui_ProgressAdvance(id2, i2);
      if (rv==GWEN_ERROR_USER_ABORTED) {
	fprintf(stderr, "Aborted by user (2)\n");
	break;
      }
    }
    GWEN_Gui_ProgressEnd(id2);

    rv=GWEN_Gui_ProgressAdvance(id1, i1);
    if (rv==GWEN_ERROR_USER_ABORTED) {
      fprintf(stderr, "Aborted by user (1)\n");
      break;
    }
  }

  GWEN_Gui_ProgressEnd(id1);

  return 0;
}*/



int test3(int argc, char **argv) {
 NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

 NSLog(@"Cocoa-Test");
 NSString *cocoa_string = @"Das ist ein Cocoa String";

 [pool release];
 return 0;
}





int main(int argc, char **argv) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSApplication *app = [NSApplication sharedApplication];
	AppDelegate *app_delegate = [[AppDelegate alloc] init];
	NSMenu *menu = [[NSMenu alloc] initWithTitle:@"Testprogramm"];
	[menu addItemWithTitle:@"Quit" action:@selector(windowWillClose:) keyEquivalent:@"Q"];
	[app setDelegate:app_delegate];
	[app setMainMenu:menu];
	[NSApp run];
	[menu release];
	[pool release];
  //test1(argc, argv);
	
  //return test2(argc, argv);
  //return test3(argc, argv);
/*#if 0
  GWEN_GUI *gui;

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  gui=Gtk2_Gui_new();
  GWEN_Gui_SetGui(gui);

  return 0;
#endif*/
}


