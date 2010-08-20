//
//  CocoaButton.h
//  
//
//  Created by Samuel Strupp on 10.08.10.
//



#ifndef COCOA_BUTTON_H
#define COCOA_BUTTON_H

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

typedef void (*gwenActionPtr)(NSButton *button, void* data);


@interface CocoaButton : NSButton <CocoaGwenGUIProtocol> {
	gwenActionPtr c_actionPtr;
	void* c_actionData;
	BOOL fillX;
	BOOL fillY;
	
	CGFloat minWidth;
}

@property BOOL fillX;
@property BOOL fillY;

-(void) setC_ActionPtr:(gwenActionPtr)ptr Data:(void*)data;

@end

#endif
