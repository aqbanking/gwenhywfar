/***************************************************************************
    begin       : Tue Aug 10 2010
    copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import <Cocoa/Cocoa.h>


@interface CocoaHLayout : NSView {
	NSMutableArray *subviewsInOrder;
}

-(void) addLayoutSubview:(NSView*)new_subview ;

@end
