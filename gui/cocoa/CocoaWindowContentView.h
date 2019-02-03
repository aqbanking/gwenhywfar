//
//  CocoaWindowContentView.h
//
//
//  Created by Samuel Strupp on 10.08.10.
//

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

@interface CocoaWindowContentView : NSView <CocoaGwenGUIProtocol>
{
  NSMutableArray *subviewsInOrder;
  BOOL fillX;
  BOOL fillY;
}

@property BOOL fillX;
@property BOOL fillY;

-(void) addLayoutSubview:(NSView *)new_subview;

@end
