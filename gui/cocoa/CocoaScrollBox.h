//
//  CocoaScrollBox.h
//  
//
//  Created by Samuel Strupp on 17.08.10.
//

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

@interface CocoaScrollBox : NSScrollView <CocoaGwenGUIProtocol> {
	//NSMutableArray *subviewsInOrder;
	BOOL fillX;
	BOOL fillY;
}

@property BOOL fillX;
@property BOOL fillY;

-(void) setLayoutedDocumentView:(NSView*)new_documentView;

@end
