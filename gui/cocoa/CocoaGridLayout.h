//
//  CocoaGridLayout.h
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

@interface CocoaGridLayout : NSView <CocoaGwenGUIProtocol> {
	NSMutableArray *subviewsInOrder;
	BOOL fillX;
	BOOL fillY;
	
	NSInteger columns;
	NSInteger rows;
}

@property BOOL fillX;
@property BOOL fillY;

@property NSInteger columns;
@property NSInteger rows;

-(void) addLayoutSubview:(NSView*)new_subview ;

@end
