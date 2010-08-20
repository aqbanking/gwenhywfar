//
//  CocoaHLineView.h
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

@interface CocoaHLineView : NSView <CocoaGwenGUIProtocol> {
	BOOL fillX;
	BOOL fillY;
}

@property BOOL fillX;
@property BOOL fillY;

@end
