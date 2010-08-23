//
//  CocoaButton.h
//  
//
//  Created by Samuel Strupp on 10.08.10.
//




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
-(NSSize) neededTextSize;

-(void) clicked:(id) sender;

@end

