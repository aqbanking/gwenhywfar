//
//  CocoaSpacer.h
//
//
//  Created by Samuel Strupp on 17.08.10.
//

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

@interface CocoaSpacer : NSView <CocoaGwenGUIProtocol>
{
  BOOL fillX;
  BOOL fillY;
}

@property BOOL fillX;
@property BOOL fillY;

@end
