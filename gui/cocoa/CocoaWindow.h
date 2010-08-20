//
//  CocoaWindow.h
//  
//
//  Created by Samuel Strupp on 20.08.10.
//



#ifndef COCOA_WINDOW_H
#define COCOA_WINDOW_H

#import <Cocoa/Cocoa.h>

typedef void (*gwenWindowActionPtr)(NSWindow *window, void* data);

@interface CocoaWindow : NSWindow {
	gwenWindowActionPtr c_actionPtr;
	void* c_actionData;
	
	gwenWindowActionPtr c_textChangedActionPtr;
	void* c_textChangedActionData;
}

@end

#endif
