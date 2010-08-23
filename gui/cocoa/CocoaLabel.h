//
//  CocoaLabel.h
//  
//
//  Created by Samuel Strupp on 10.08.10.
//



#ifndef COCOA_LABEL_H
#define COCOA_LABEL_H

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

@interface CocoaLabel : NSTextField <CocoaGwenGUIProtocol> {
	BOOL fillX;
	BOOL fillY;
	
	CGFloat minWidth;
}

@property BOOL fillX;
@property BOOL fillY;

-(NSSize) neededTextSize;

@end

#endif
